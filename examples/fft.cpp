// Modified copy of the bspedupack implementation of
// the fast fourier transform, updated to use bulk.
//
// At the bottom of the file, the `old` C code is given,
// in order to see the difference with the new C++ code.

#include "bulk/bulk.hpp"
#include "bulk/util/timer.hpp"
#include <cmath>
#include <complex>
#include <fftw3.h>

#include "set_backend.hpp"

using NumType = std::complex<double>;

// Perform NITERS forward and backward transforms.
// A large NITERS helps to obtain accurate timings.
constexpr int NITERS = 40;
// Print NPRINT values per processor
constexpr int NPRINT = 0;
constexpr double MEGA = 1000000.0;

// estimate, measure or patient
constexpr auto FFTW_PLAN_MODE = FFTW_MEASURE; // 30-minute benchmark mode: FFTW_PATIENT


double fftw_sequential_test(int n);
std::pair<double, double> bspfft_test(bulk::world &world, int n);

int main() {
    auto report = bulk::util::table("FFT time", "size");
    report.columns("p", "method", "time");

    for (int power = 23; power <= 26; ++power) {
        int n = 1 << power;
        double time = fftw_sequential_test(n);
        report.row(std::to_string(n), 1, "FFTW sequential", time);

        environment env;
        for (int p = 1; p <= env.available_processors(); p *= 2) {
            env.spawn(p, [&report, n, p](bulk::world &world) {
                auto times = bspfft_test(world, n);
                if (world.rank() == 0) {
                    report.row(std::to_string(n), p, "Yzelman", times.first);
                    report.row(std::to_string(n), p, "Paper", times.second);
                }
            });
        }
    }

    printf("\n\n Results:\n\n");
    std::cout << report.print() << std::endl;

    return 0;
}

double fftw_sequential_test(int n) {
    printf("Sequential FFT  of length %d using FFTW, doing %d benchmark iterations.\n", n, NITERS);

    std::vector<NumType> xs(n);

    printf("Initializing FFTW.\n");
    bulk::util::timer timerinit;
    fftw_complex* xs_fftw = (fftw_complex*)&xs[0];
    auto plan_fwd = fftw_plan_dft_1d(n, xs_fftw, xs_fftw, FFTW_FORWARD, FFTW_PLAN_MODE);
    auto plan_bwd = fftw_plan_dft_1d(n, xs_fftw, xs_fftw, FFTW_BACKWARD, FFTW_PLAN_MODE);
    double inittime = timerinit.get<std::ratio<1>>();
    printf("Initializing done in %f sec.\n", inittime);

    // Initialize array
    using namespace std::complex_literals;
    for (int j = 0; j < n; ++j) {
        xs[j] = (double)j + 1i;
    }

    // Compute the fft
    // Perform and time the FFTs
    bulk::util::timer timer1;
    for (int it = 0; it < NITERS; it++) {
        fftw_execute(plan_fwd);
        fftw_execute(plan_bwd);
        double ninv = 1.0 / (double)n;
        for (auto& x : xs)
            x *= ninv;
    }
    double ffttime = timer1.get<std::ratio<1>>();

    
    // For debug purposes: execute FFT once to check numbers
    if (NPRINT) {
        fftw_execute(plan_fwd);
        printf("After forward FFT, first 4 components are:\n");
        printf("j = 0   Re = %f Im = %f\n", xs[0].real(), xs[0].imag());
        printf("j = 1   Re = %f Im = %f\n", xs[1].real(), xs[1].imag());
        printf("j = 2   Re = %f Im = %f\n", xs[2].real(), xs[2].imag());
        printf("j = 3   Re = %f Im = %f\n", xs[3].real(), xs[3].imag());
        fftw_execute(plan_bwd);
        double ninv = 1.0 / (double)n;
        for (auto &x : xs)
            x *= ninv;
    }

    fftw_destroy_plan(plan_fwd);
    fftw_destroy_plan(plan_bwd);

    ffttime = ffttime / (2.0 * NITERS);
    printf("Time per FFT = %lf sec\n", ffttime);
    return ffttime;
}


// Taken from bspedupack and updated to C++
template<bool useFFTW = false, bool bookVersion = false>
class BulkFFT {
  public:
    BulkFFT(bulk::world& world, int n_) {
        n = n_;
        p = world.active_processors();
        s = world.rank();
        if (!isPowerOfTwo(n) || !isPowerOfTwo(p)) {
            world.log("ERROR: BulkFFT requires n and p to be powers of 2.");
            world.abort();
        }
        fftw_xs = nullptr;
        bspfft_init();
    }
    ~BulkFFT() {
        if (useFFTW) {
            fftw_destroy_plan(plan_consec_fwd);
            fftw_destroy_plan(plan_consec_bwd);
            fftw_destroy_plan(plan_np_fwd);
            fftw_destroy_plan(plan_np_bwd);
        }
    }

    // Re-initialize to a different size
    void reinitialize(int n_) {
        n = n_;
        bspfft_init();
    }

    // THIS WILL OVERWRITE THE CONTENTS OF xs !
    void fftw_initialize(int n_, bulk::coarray<NumType>& xs) {
        reinitialize(n_);
        // Parameters to fftw plan
        // int rank,            -- 1D fft
        // const int *n,        -- lengths of each fft: {k1, k1, ... }
        // int howmany,         -- number of ffts
        // fftw_complex *in,    -- input array
        // const int *inembed,  -- n.a.
        // int istride,         -- j-th element is at in[ j * istride ]
        // int idist,           -- j-th FFT starts at in[ j * idist ]
        // fftw_complex *out,   -- output array, can be equal to input
        // const int *onembed,  -- n.a.
        // int ostride,         -- output stride
        // int odist,           -- output dist
        // int sign,            -- FFTW_FORWARD
        // unsigned flags       -- whether to time different methods

        int np = n / p;

        int howmany = np / k1;
        std::vector<int> sizes(howmany, k1);
        fftw_xs = (fftw_complex*)&xs[0];

        // FFTW plan creation is NOT thread safe!!
        for (int i = 0; i < p; ++i) {
            if (i == s) {
                plan_consec_fwd = fftw_plan_many_dft(
                    1, &sizes[0], howmany, fftw_xs, NULL, 1, k1, fftw_xs, NULL,
                    1, k1, FFTW_FORWARD, FFTW_PLAN_MODE);
                plan_np_fwd = fftw_plan_dft_1d(np, fftw_xs, fftw_xs,
                                               FFTW_FORWARD, FFTW_PLAN_MODE);
                plan_consec_bwd = fftw_plan_many_dft(
                    1, &sizes[0], howmany, fftw_xs, NULL, 1, k1, fftw_xs, NULL,
                    1, k1, FFTW_BACKWARD, FFTW_PLAN_MODE);
                plan_np_bwd = fftw_plan_dft_1d(np, fftw_xs, fftw_xs,
                                               FFTW_BACKWARD, FFTW_PLAN_MODE);
            }
            xs.world().sync();
        }
    }

    // Fast Fourier Transform
    // The coarray xs must be of size n/p on each core and must
    // be distributed amongst processors in a cyclic distribution:
    // on core s the j-th element is the (j * p + s)-th global element.
    // The output will have the same distribution.
    template <bool Forward>
    void fft(bulk::coarray<NumType>& xs) {
        if ((int)xs.size() != n / p) {
            xs.world().log(
                "ERROR: BulkFFT::fft called on coarray of invalid size.");
            xs.world().abort();
            return;
        }
        if (useFFTW) {
            if (fftw_xs != (fftw_complex*)&xs[0]) {
                xs.world().log("ERROR: BulkFFT::fft called with FFTW without first calling fftw_initialize.");
                xs.world().abort();
                return;
            }
        }
        if (bookVersion)
            return bspfft_paper<Forward>(xs);
        else
            bspfft<Forward>(xs);
    }

  private:
    //
    // Initialization functions
    //
    static bool isPowerOfTwo(int n) { return (n & (n - 1)) == 0; }

    // This parallel function initializes all the tables used in the FFT.
    void bspfft_init() {
        int np = n / p;

        // This computes the largest butterfly size k1 of the first
        // superstep in a parallel FFT of length n on p processors with p < n.
        int c;
        for (c = 1; c < p; c *= np)
            ;
        k1 = n / c;

        // Initialize bit reveral permutations
        rho_np.resize(np);
        rho_p.resize(p);
        bitrev_init(rho_np);
        bitrev_init(rho_p);

        // Initialize weights
        ufft_init(k1, w0);
        ufft_init(np, w);

        // Compute size of `tw` weights
        int count = 0;
        for (c = k1; c <= p; c *= np)
            count++;
        tw.resize(count * np);
        auto tw_cur = tw.begin();
        for (c = k1; c <= p; c *= np) {
            double alpha = (s % c) / (double)(c);
            // Initialize the weights tw_cur,...,tw_cur+np
            // to values  tw_cur[j] = exp(-2 pi i rho_np(j) alpha / np)
            double theta = -2.0 * M_PI * alpha / double(np);
            for (auto rho : rho_np) {
                *tw_cur++ = std::polar(1.0, rho * theta);
            }
        }
        
        // Compute the book-edition weights
        {
            // First a dry-run to get the size
            auto c = 1;
            auto k = 2;
            auto start = 0;
            while (c <= p) {
                while (k <= np * c) {
                    start += k / (2 * c);
                    k *= 2;
                }
                if (c < p)
                    c = p;
                else
                    c = 2 * p;
            }

            wbook.resize( start );
            c = 1;
            k = 2;
            start = 0;
            while (c <= p) {
                auto j0 = s % c;
                while (k <= np * c) {
                    double theta = -2.0 * M_PI / double(k);
                    for (auto j = 0; j < k / (2 * c); j++) {
                        double jtheta = (j0 + j * c) * theta;
                        wbook[start++] = std::polar(1.0, jtheta);
                    }
                    k *= 2;
                }
                if (c < p)
                    c = p;
                else
                    c = 2 * p;
            }
        }
        // Compute the paper-edition weights
        {
            // Dry-run for size
            auto size = 0u;
            for (int k = 2 * np; k <= n; k *= 2) {
                size += n / (2 * p); // = b * k2p;
            }
            wpaper.resize(size);

            auto start = 0u;
            for (int k = 2 * np; k <= n; k *= 2) {
                int b = n / k;
                auto k2p = k / 2 / p;
                for (auto r = 0; r < b; ++r) {
                    double theta = -2.0 * M_PI / double(k);
                    for (auto jprime = 0; jprime < k2p; ++jprime) {
                        // exp(-2pi i * j/k)
                        auto j = s + jprime * p;
                        wpaper[start++] = std::polar(1.0, j * theta);
                    }
                }
            }
        }
    }

    // Initializes the bit-reversal permutation rho
    // of lenght n that must satisfy n=2^m, m>= 0.
    static void bitrev_init(std::vector<unsigned int>& rho) {
        auto n = rho.size();
        if (n == 1) {
            rho[0] = 0;
            return;
        }

        for (auto j = 0u; j < n; ++j) {
            auto val = 0u; // val becomes j but bit-reversed
            auto remaining = j;
            for (auto k = 1u; k < n; k <<= 1) {
                val <<= 1;              // shift left by one
                val |= (remaining & 1); // append the next bit of j
                remaining >>= 1;
            }
            rho[j] = val;
        }
    }

    // Initialize the weights to exp(-2 pi i j / n )
    // for 0 <= j < n/2 where n = 2^m, m >= 0.
    // So the array length is n/2 !
    static void ufft_init(unsigned int n, std::vector<NumType>& ws) {
        auto len = n / 2;
        ws.resize(len);
        // w[j] = exp(-(2*pi*i)*j/n)
        auto theta = -2.0 * M_PI / (double)n;
        for (auto j = 0u; j < len; j++)
            ws[j] = std::polar(1.0, j * theta);
    }

    //
    // FFT functions
    //

    // This sequential function computes the unordered discrete Fourier
    // transform of a complex vector x of length n. n=2^m, m >= 0.
    // If Forward, then the forward unordered dft FRx is computed;
    // else, the backward unordered dft conjg(F)Rx is computed,
    // where F is the n by n Fourier matrix and R the n by n bit-reversal
    // matrix. The output overwrites x.
    // w is a table of n/2 complex weights
    //    exp(-2*pi*i*j/n), 0 <= j < n/2,
    // which must have been initialized before calling this function.
    // ---
    // The reason for xs not being given as std::vector is that
    // ufft is called on subvectors of a larger vector and this would
    // require copying because C++ does not have support for
    // slices/views/ranges and so on.
    template <bool Forward>
    static void ufft(unsigned int n, NumType* xs,
                     const std::vector<NumType>& ws) {
        // assume ws.size == n/2
        for (auto k = 2u; k <= n; k *= 2) {
            auto nk = n / k;
            for (auto r = 0u; r < nk; r++) {
                auto rk = r * k;
                for (auto j = 0u; j < k / 2; j++) {
                    auto w = ws[j * nk];
                    if (Forward == false)
                        w = std::conj(w);
                    auto j0 = rk + j;
                    auto j2 = j0 + k / 2;
                    auto tau = w * xs[j2];
                    xs[j2] = xs[j0] - tau;
                    xs[j0] += tau;
                }
            }
        }
    }

    // Multiply the vector xs componentwise by vector ws or conjg(ws)
    // depending on the template parameter.
    // The result overwrites xs
    template <bool Forward>
    static void twiddle(bulk::coarray<NumType> &xs, NumType *ws) {
        if (Forward) {
            for (auto &x : xs) {
                x *= *ws++;
            }
        } else {
            for (auto &x : xs) {
                x *= std::conj(*ws++);
            }
        }
    }

    // This sequential function permutes a complex vector xs
    // by the permutation sigma: xs[j] <- xs[sigma[j]]
    // This is *NOT* for general permutations sigma,
    // only for the ones generated by the bitrev_init functions
    static void permute(NumType* xs, const std::vector<unsigned int>& sigma) {
        for (auto j = 0u; j < sigma.size(); ++j) {
            auto sj = sigma[j];
            if (j < sj) {
                std::swap(xs[j], xs[sj]);
            }
        }
    }

    //
    // Parallel functions
    //

    void bspredistr(bulk::coarray<NumType>& xs, int c0, int c1, bool reversed) {

        // This function redistributes the complex vector x of length n,
        // from group-cyclic distribution
        // over p processors with cycle c0 to cycle c1, where
        // c0, c1, p, n are powers of two with 1 <= c0 <= c1 <= p <= n.
        // s is the processor number, 0 <= s < p.
        // If reversed=true, the function assumes the processor numbering
        // is bit reversed on input.
        // rho_p is the bit-reversal permutation of length p.

        int np = n / p;
        int ratio = c1 / c0;
        int size = std::max(np / ratio, 1);
        int npackets = np / size;

        int j0, j2;

        if (reversed) {
            j0 = rho_p[s] % c0;
            j2 = rho_p[s] / c0;
        } else {
            j0 = s % c0;
            j2 = s / c0;
        }

        std::vector<NumType> tmp(size);
        for (auto j = 0; j < npackets; j++) {
            for (int r = 0; r < size; r++) {
                tmp[r] = xs[j + r * ratio];
            }
            int jglob = j2 * c0 * np + j * c0 + j0;
            int destproc = (jglob / (c1 * np)) * c1 + jglob % c1;
            int destindex = (jglob % (c1 * np)) / c1;

            xs(destproc)[{destindex, destindex + size}] = tmp;
        }
        xs.world().sync();
    }

    template <bool Forward>
    void bspfft(bulk::coarray<NumType>& xs) {
        // This parallel function computes the discrete Fourier transform
        // of a complex array x of length n=2^m, m >= 1.
        // p is the number of processors, p=2^q, 0 <= q < m.
        // s is the processor number, 0 <= s < p.
        // The function uses three weight tables:
        //     w0 for the unordered fft of length k1,
        //     w  for the unordered fft of length n/p,
        //     tw for a number of twiddles, each of length n/p.
        // The function uses two bit-reversal permutations:
        //     rho_np of length n/p,
        //     rho_p of length p.
        // The weight tables and bit-reversal permutations must have been
        // initialized before calling this function.
        // If Forward = true, then the dft is computed,
        //   y[k] =      sum j=0 to n-1 exp(-2*pi*i*k*j/n)*x[j], for 0 <= k < n.
        // If Forward = false, then the inverse dft is computed,
        //   y[k] = (1/n)sum j=0 to n-1 exp(+2*pi*i*k*j/n)*x[j], for 0 <= k < n.
        // Here, i=sqrt(-1). The output vector y overwrites xs.

        // Benchmark result on n=2^22, all times in seconds
        // Sequential FFTW time: 0.24; 0.22; 0.23
        // With p=4, time of everything up to and including:
        // - permute -> 0.056 ; 0.057
        // - ufft    -> 0.073 ; 0.080 (no fftw)
        // - ufft    -> 0.086 ; 0.086 (fftw)  !! permute is expensive
        // - redistr -> 0.167 ; 0.164 (no fftw)
        // - redistr -> 0.187 ; 0.179 (fftw)
        // - twiddle -> 0.187 ; 0.209 ; 0.186 (no fftw)
        // - twiddle -> 0.202 ; 0.199 ; 0.199 (fftw)
        // - ufft2   -> 0.771 ; 0.794 (no fftw)
        // - ufft2   -> 0.372 ; 0.358 (fftw)
        //
        // all steps but  without redistr:
        // no fftw: 0.686
        // fftw: 0.266 <-- so even permute+fftw kernels and NO communication is slower than sequential FFTW
        //
        // all steps but no redistr and no twiddle
        // no fftw: 0.657
        // fftw: 0.255
        //
        // n=2^25, p=4:
        // - sequential: 2.53
        // - no fftw: 7.57
        // - fftw: 3.11
        //
        // n=2^26, p=4:
        // - sequential: 5.73
        // - no fftw: 20.74
        // - fftw: 8.64

        int np = n / p;
        permute(xs.begin(), rho_np);

        if (useFFTW) {
            // Taken from Yzelman bsp paper
            // Partially undo the permutation
            // Can not easiliy combine the rho_np permutation
            // with this one because it will no longer be swaps
            for (int r = 0; r < np / k1; r++)
                permute(xs.begin() + r * k1, rho_p);
            if (Forward)
                fftw_execute(plan_consec_fwd);
            else
                fftw_execute(plan_consec_bwd);
        } else {
            for (int r = 0; r < np / k1; r++)
                ufft<Forward>(k1, xs.begin() + r * k1, w0);
        }

        int c0 = 1;
        bool rev = true;
        NumType* tw_cur = &tw[0];
        for (int c = k1; c <= p; c *= np) {
            bspredistr(xs, c0, c, rev);
            rev = false;
            c0 = c;

            twiddle<Forward>(xs, tw_cur);
            tw_cur += np;

            if (useFFTW) {
                // Undo permutation
                permute(xs.begin(), rho_np);
                if (Forward)
                    fftw_execute(plan_np_fwd);
                else
                    fftw_execute(plan_np_bwd);
            } else {
                ufft<Forward>(np, xs.begin(), w);
            }
        }

        if (Forward == false) {
            double ninv = 1 / (double)n;
            for (auto& x : xs)
                x *= ninv;
        }
    }

    // ONLY WORKS FOR n > p^2 !!
    template <bool Forward>
    void bspfft_paper(bulk::coarray<NumType>& xs) {
        int np = n / p;

        if (Forward)
            fftw_execute(plan_np_fwd);
        else
            fftw_execute(plan_np_bwd);

        // n=2^22, p=4. (sequential 0.23 sec)
        // Time till here: 0.10 sec

        bspredistr(xs, 1, p, true);

        // Time till here: 0.195 sec

#if 1
        // From paper: (s0 = s, s1 = 0)
        // Weights have been precomputed such that we need all of them only once
        auto curWeight = wpaper.begin();
        for (int k = 2*np; k <= n; k *= 2) {
            // Compute butterfly k
            for (auto r = 0; r < (n / k); ++r) {
                auto rkp = r * k / p;
                auto k2p = k / 2 / p;
                //for (auto j = s; j < s + (k/2); j += p) { // from paper
                for (auto j = 0; j < k2p; j++) { // from book
                    auto w = *curWeight++;
                    if (Forward == false)
                        w = std::conj(w);
                    auto j0 = rkp + j;
                    auto j2 = j0 + k2p;;
                    auto tau = w * xs[j2];
                    xs[j2] = xs[j0] - tau;
                    xs[j0] += tau;
                }
            }
        }
#else
        // From book: inner j index is different from paper version
        auto start = np - 1;
        for (int k = 2*np; k <= n; k *= 2) {
            //butterfly_stage(xs, np, k/p, forward, &wbook[start]);
            {
                auto k2p = k/2/p;
                // np / (k/p) = n/k
                for (auto r = 0; r < n/k; ++r) {
                    auto rkp = r * k / p;
                    for (auto j = 0; j < k2p; j++) {
                        auto w = wbook[start+j];
                        if (Forward == false)
                            w = std::conj(w);
                        auto j0 = rkp + j;
                        auto j2 = j0 + k2p;
                        auto tau = w * xs[j2];
                        xs[j2] = xs[j0] - tau;
                        xs[j0] += tau;
                    }
                }

            }
            start += k/(2*c);
        }
#endif

        if (Forward == false) {
            double ninv = 1 / (double)n;
            for (auto& x : xs)
                x *= ninv;
        }

        // Total time: crashed
    }

    //
    // Internal variables
    //

    // Variables used by most fft related functions
    int n;
    int p;
    int s;
    int k1;

    // Weights, some differ per core, some are the same
    std::vector<NumType> w0;
    std::vector<NumType> w;
    std::vector<NumType> tw;
    std::vector<NumType> wbook;
    std::vector<NumType> wpaper;
    // Permutations
    std::vector<unsigned int> rho_np;
    std::vector<unsigned int> rho_p;
    // FFTW
    fftw_plan plan_consec_fwd;
    fftw_plan plan_np_fwd;
    fftw_plan plan_consec_bwd;
    fftw_plan plan_np_bwd;
    fftw_complex* fftw_xs;
};

//  A Fast Fourier Transform and its inverse.
//  This is a test program which uses bspfft
//
//  The input vector is defined by x[j]=j+i, for 0 <= j < n.
//  Here i= sqrt(-1).
//
//  The output vector should equal the input vector,
//  up to roundoff errors. Output is by triples (j, Re x[j], Im x[j]).
//  Warning: don't rely on this test alone to check correctness.
//  (After all, deleting the main loop will give similar results ;)

template<bool useFFTW = false, bool bookVersion = false>
double bspfft_test_internal(bulk::world& world, int n) {
    int s = world.rank();
    int p = world.active_processors();

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(s, &cpuset);
    int rc = pthread_setaffinity_np(pthread_self(),
                                    sizeof(cpu_set_t), &cpuset);

    if (s == 0) {
        if (useFFTW)
            if(bookVersion)
                world.log("Parallel FFT, book version, with FFTW kernels, "
                          "using %d processors, doing %d benchmark iterations",
                          p, NITERS);
            else
                world.log("Parallel FFT, Yzelman version, with FFTW kernels, "
                          "using %d processors, doing %d benchmark iterations",
                          p, NITERS);
        else
            world.log("Parallel FFT, Yzelman version, without FFTW kernels, "
                      "using %d processors, doing %d benchmark iterations",
                      p, NITERS);
    }

    int np = n / p;
    bulk::coarray<NumType> xs(world, np, 0.0);

    BulkFFT<useFFTW, bookVersion> bulkfft(world, n);

    if (useFFTW) {
        if (s == 0)
            world.log("Initializing FFTW.");
        world.sync();

        bulkfft.fftw_initialize(n, xs); // This overwrites contents of xs !

        if (s == 0)
            world.log("FFTW initialized.");
        world.sync();
    }

    // Time the normal initialization
    bulk::util::timer timer0;
    for (int it = 0; it < NITERS; it++) {
        bulkfft.reinitialize(n);
    }
    world.sync();
    double init_time = timer0.get<std::ratio<1>>();

    // Initialize the coarray
    using namespace std::complex_literals;
    for (int j = 0; j < np; j++) {
        int jglob = j * p + s;
        xs[j] = (double)jglob + 1i;
    }
    world.sync();

    // Perform and time the FFTs
    bulk::util::timer timer1;
    for (int it = 0; it < NITERS; it++) {
        bulkfft.template fft<true> (xs);
        bulkfft.template fft<false> (xs);
    }
    world.sync();
    double ffttime = timer1.get<std::ratio<1>>();

    // Compute the accuracy
    double max_error = 0.0;
    for (int j = 0; j < np; j++) {
        int jglob = j * p + s;
        double error = std::abs(xs[j] - ((double)jglob + 1i));
        if (error > max_error)
            max_error = error;
    }
    // Send errors to core 0
    bulk::coarray<double> Error(world, p);
    Error(0)[s] = max_error;
    world.sync();

    if (s == 0) {
        max_error = 0.0;
        for (auto e : Error) {
            if (e > max_error) {
                max_error = e;
            }
        }
    }

    if (NPRINT) {
        bulkfft.template fft<true>(xs);
        for (int j = 0; j < NPRINT && j < np; j++) {
            int jglob = j * p + s;
            world.log("FFT[xs] proc=%d j=%d Re= %f Im= %f", s, jglob,
                      xs[j].real(), xs[j].imag());
        }
    }
    world.sync();

    if (s == 0) {
        double nflops;
        world.log("Time per initialization = %lf sec", init_time / NITERS);
        ffttime = ffttime / (2.0 * NITERS);
        world.log("Time per FFT = %lf sec", ffttime);
        nflops = 5 * n * log((double)n) / log(2.0) + 2 * n;
        world.log("Computing rate in FFT = %lf Mflop/s",
                  nflops / (MEGA * ffttime));
        world.log("Absolute error= %e", max_error);
        world.log("Relative error= %e", max_error / n);
    }
    world.sync();

    return ffttime;
}

std::pair<double, double> bspfft_test(bulk::world &world, int n) {
    //bspfft_test_internal<false,false>(world, n);
    double t1 = bspfft_test_internal<true, false>(world, n);
    double t2 = bspfft_test_internal<true, true>(world, n);
    return {t1, t2};
}
