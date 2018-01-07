// Modified copy of the bspedupack implementation of
// the fast fourier transform, updated to use bulk.
//
// At the bottom of the file, the `old` C code is given,
// in order to see the difference with the new C++ code.

#include <cmath>
#include <complex>
#include "bulk/bulk.hpp"
#include "bulk/util/timer.hpp"

#include "set_backend.hpp"

int k1_init(int n, int p);
void bspfft_test(bulk::world& world, int n);
void bspfft_test_new(bulk::world& world, int n);

int main() {
    environment env;

    env.spawn(env.available_processors(),
              [](bulk::world& world) { bspfft_test_new(world, 128 * 4096); });
    return 0;
}

// Taken from bspedupack and updated to C++
using NumType = std::complex<double>;

//
// Sequential functions
//



// This sequential function computes the unordered discrete Fourier
// transform of a complex vector x of length n. n=2^m, m >= 0.
// If sign = 1, then the forward unordered dft FRx is computed;
// if sign =-1, the backward unordered dft conjg(F)Rx is computed, 
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
void ufft(unsigned int n, NumType* xs, const std::vector<NumType>& ws) {
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


// Initialize the weights to exp(-2 pi i j / n )
// for 0 <= j < n/2 where n = 2^m, m >= 0.
// So the array length is n/2 !
void ufft_init(unsigned int n, std::vector<NumType>& ws) {
    auto len = n/2;
    ws.resize(len);
    // w[j] = exp(-(2*pi*i)*j/n)
    auto theta = -2.0 * M_PI / (double)n;
    for (auto j = 0u; j < len; j++)
        ws[j] = std::polar(1.0, j * theta);
}

// Multiply the vector xs componentwise by vector ws or conjg(ws)
// depending on the template parameter.
// The result overwrites xs
template <bool Forward>
void twiddle(bulk::coarray<NumType>& xs, NumType* ws) {
    for (auto& x : xs) {
        x = x * (Forward ? (*ws) : std::conj(*ws));
        ws++;
    }
}

// This sequential function initializes the weights vector ws
// to be used in twiddling.
// Alpha is a real shift parameter.
// rho is the bit-reversal permutation of length n
// which must have been initialized before calling this function.
// The output is, for 0 <= j < n
//     ws[j] = exp(-2 pi i rho(j) alpha / n)
void twiddle_init(double alpha, const std::vector<unsigned int>& rho, NumType* ws) {
    auto n = rho.size();
    double theta = -2.0 * M_PI * alpha / double(n);
    for (auto j = 0u; j < n; ++j) {
        ws[j] = std::polar(1.0, rho[j] * theta);
    }
}

// This sequential function permutes a complex vector xs
// by the permutation sigma: xs[j] <- xs[sigma[j]]
// This is *NOT* for general permutations sigma!
// The permutation sigma must be decomposable into disjoint swaps
// in a particular way
void permute(bulk::coarray<NumType>& xs,
             const std::vector<unsigned int>& sigma) {
    if (xs.size() != sigma.size())
        return;

    for (auto j = 0u; j < xs.size(); ++j) {
        auto sj = sigma[j];
        if (j < sj) {
            std::swap(xs[j], xs[sj]);
        }
    }
}

// Initializes the bit-reversal permutation rho
// of lenght n that must satisfy n=2^m, m>= 0.
void bitrev_init(std::vector<unsigned int>& rho) {
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

//
// Parallel functions
//

void bspredistr(bulk::coarray<NumType>& xs, int n, int p, int s, int c0, int c1,
                bool reversed, std::vector<unsigned int>& rho_p) {

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

    std::vector<NumType> tmp(size);

    int j0, j2;

    if (reversed) {
        j0 = rho_p[s] % c0;
        j2 = rho_p[s] / c0;
    } else {
        j0 = s % c0;
        j2 = s / c0;
    }
    for (auto j = 0; j < npackets; j++) {
        int jglob = j2 * c0 * np + j * c0 + j0;
        int destproc = (jglob / (c1 * np)) * c1 + jglob % c1;
        int destindex = (jglob % (c1 * np)) / c1;
        for (int r = 0; r < size; r++) {
            tmp[r] = xs[j + r * ratio];
        }
        xs.put(destproc, {destindex, (destindex + size)}, tmp);
    }
    xs.world().sync();
}


template <bool Forward>
void bspfft(bulk::coarray<NumType>& xs, int n, int p, int s,
            std::vector<NumType>& w0, std::vector<NumType>& w,
            std::vector<NumType>& tw, std::vector<unsigned int>& rho_np,
            std::vector<unsigned int>& rho_p) {

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
    //     y[k] =      sum j=0 to n-1 exp(-2*pi*i*k*j/n)*x[j], for 0 <= k < n.
    // If Forward = false, then the inverse dft is computed,
    //     y[k] = (1/n)sum j=0 to n-1 exp(+2*pi*i*k*j/n)*x[j], for 0 <= k < n.
    // Here, i=sqrt(-1). The output vector y overwrites xs.

    int np, k1, r, c0, c;

    np= n/p;
    k1= k1_init(n,p);
    permute(xs, rho_np);
    for(r=0; r<np/k1; r++)
        ufft<Forward>(k1, xs.begin() + r * k1, w0);

    c0= 1;
    bool rev = true;
    NumType* tw_cur = &tw[0];
    for (c=k1; c<=p; c *=np){
        bspredistr(xs, n, p, s, c0, c, rev, rho_p);
        rev = false;
        c0= c;

        twiddle<Forward>(xs, tw_cur);
        tw_cur += np;

        ufft<Forward>(np, xs.begin(), w);
    }

    if (Forward == false) {
        double ninv= 1 / (double)n;
        for (auto& x : xs)
            x *= ninv;
    }
}

// This parallel function initializes all the tables used in the FFT.
void bspfft_init(int n, int p, int s, std::vector<NumType>& w0,
                 std::vector<NumType>& w, std::vector<NumType>& tw,
                 std::vector<unsigned int>& rho_np,
                 std::vector<unsigned int>& rho_p) {
    int np = n / p;

    rho_np.resize(np);
    rho_p.resize(p);
    bitrev_init(rho_np);
    bitrev_init(rho_p);

    int k1 = k1_init(n, p);

    ufft_init(k1, w0);
    ufft_init(np, w);

    NumType* tw_cur = &tw[0];
    for (int c = k1; c <= p; c *= np) {
        double alpha = (s % c) / (double)(c);
        twiddle_init(alpha, rho_np, tw_cur);
        tw_cur += np;
    }
}


//  a Fast Fourier Transform and its inverse.
//  This is a test program which uses bspfft to perform 
//
//  The input vector is defined by x[j]=j+i, for 0 <= j < n.
//  Here i= sqrt(-1).
//
//  The output vector should equal the input vector,
//  up to roundoff errors. Output is by triples (j, Re x[j], Im x[j]).
//  Warning: don't rely on this test alone to check correctness. 
//  (After all, deleting the main loop will give similar results ;) 


#define NITERS 50  // Perform NITERS forward and backward transforms.
                   // A large NITERS helps to obtain accurate timings.
#define NPRINT 0   // Print NPRINT values per processor
#define MEGA 1000000.0

void bspfft_test_new(bulk::world& world, int n) {
    int s = world.rank();
    int p = world.active_processors();

    if (s == 0) {
        world.log("FFT of vector of length %d using %d processors", n, p);
        world.log("performing %d forward and %d backward transforms", NITERS,
                  NITERS);
    }

    int np = n / p;
    bulk::coarray<NumType> x(world, np, 0.0);
    bulk::coarray<double> Error(world, p);

    int k1 = k1_init(n, p);
    std::vector<NumType> w0(k1);
    std::vector<NumType> w(np);
    std::vector<NumType> tw(2 * np + p); // TODO: size ?
    std::vector<unsigned int> rho_np(np);
    std::vector<unsigned int> rho_p(p);

    int q, jglob, it;
    double time0, time1, ffttime, nflops, max_error;

    // Initialize x
    using namespace std::complex_literals;
    for (int j = 0; j < np; j++) {
        jglob = j * p + s;
        x[j] = (double)jglob + 1i;
    }

    world.sync();

    bulk::util::timer timer0;
    // Initialize the weight and bit reversal tables
    for (it = 0; it < NITERS; it++) {
        bspfft_init(n, p, s, w0, w, tw, rho_np, rho_p);
    }
    world.sync();
    time0 = timer0.get();

    // Perform the FFTs
    bulk::util::timer timer1;
    for (it = 0; it < NITERS; it++) {
        bspfft<true> (x, n, p, s, w0, w, tw, rho_np, rho_p);
        bspfft<false>(x, n, p, s, w0, w, tw, rho_np, rho_p);
    }
    world.sync();
    time1 = timer1.get();

    // Compute the accuracy
    max_error = 0.0;
    for (int j = 0; j < np; j++) {
        jglob = j * p + s;
        double error = std::abs(x[j] - ((double)jglob + 1i));
        if (error > max_error)
            max_error = error;
    }
    Error(0)[s] = max_error;
    world.sync();

    if (s == 0) {
        max_error = 0.0;
        for (q = 0; q < p; q++) {
            if (Error[q] > max_error)
                max_error = Error[q];
        }
    }

    for (int j = 0; j < NPRINT && j < np; j++) {
        jglob = j * p + s;
        world.log("proc=%d j=%d Re= %f Im= %f", s, jglob, x[2 * j],
                  x[2 * j + 1]);
    }
    world.sync();

    if (s == 0) {
        world.log("Time per initialization = %lf sec", time0 / NITERS);
        ffttime = time1 / (2.0 * NITERS);
        world.log("Time per FFT = %lf sec", ffttime);
        nflops = 5 * n * log((double)n) / log(2.0) + 2 * n;
        world.log("Computing rate in FFT = %lf Mflop/s",
                  nflops / (MEGA * ffttime));
        world.log("Absolute error= %e", max_error);
        world.log("Relative error= %e\n", max_error / n);
    }
    world.sync();
}




/****************** Sequential functions ********************************/
void ufft(double* x, int n, int sign, double* w) {

    /* This sequential function computes the unordered discrete Fourier
       transform of a complex vector x of length n, stored in a real array
       of length 2n as pairs (Re x[j], Im x[j]), 0 <= j < n.
       n=2^m, m >= 0.
       If sign = 1, then the forward unordered dft FRx is computed;
       if sign =-1, the backward unordered dft conjg(F)Rx is computed, 
       where F is the n by n Fourier matrix and R the n by n bit-reversal
       matrix. The output overwrites x.
       w is a table of n/2 complex weights, stored as pairs of reals,
          exp(-2*pi*i*j/n), 0 <= j < n/2,
       which must have been initialized before calling this function.
    */
    
    int k, nk, r, rk, j, j0, j1, j2, j3;
    double wr, wi, taur, taui;

    for(k=2; k<=n; k *=2){
        nk= n/k;
        for(r=0; r<nk; r++){
            rk= 2*r*k;
            for(j=0; j<k; j +=2){
                wr= w[j*nk];
                if (sign==1) {
                    wi= w[j*nk+1];
                } else {
                    wi= -w[j*nk+1];
                }
                j0= rk+j;
                j1= j0+1;
                j2= j0+k;
                j3= j2+1;
                taur= wr*x[j2] - wi*x[j3];
                taui= wi*x[j2] + wr*x[j3];
                x[j2]= x[j0]-taur;   
                x[j3]= x[j1]-taui;   
                x[j0] += taur;   
                x[j1] += taui;   
            }
        }
    }

} /* end ufft */

void ufft_init(int n, double *w){

    /* This function initializes the n/2 weights to be used
       in a sequential radix-2 FFT of length n.
       n=2^m, m >= 0.
       w is a table of n/2 complex weights, stored as pairs of reals,
          exp(-2*pi*i*j/n), 0 <= j < n/2.
    */

    int j, n4j, n2j;
    double theta;

    if (n==1)
        return;
    theta= -2.0 * M_PI / (double)n;
    w[0]= 1.0;
    w[1]= 0.0;
    if (n==4){ 
        w[2]=  0.0;
        w[3]= -1.0;
    } else if (n>=8) {
        /* weights 1 .. n/8 */
        for(j=1; j<=n/8; j++){
            w[2*j]=   cos(j*theta);
            w[2*j+1]= sin(j*theta);
        }
        /* weights n/8+1 .. n/4 */
        for(j=0; j<n/8; j++){
            n4j= n/4-j;
            w[2*n4j]=   -w[2*j+1];
            w[2*n4j+1]= -w[2*j];
        }
        /* weights n/4+1 .. n/2-1 */
        for(j=1; j<n/4; j++){
            n2j= n/2-j;
            w[2*n2j]=   -w[2*j];
            w[2*n2j+1]=  w[2*j+1];
        }
    }

} /* end ufft_init */

void twiddle(bulk::coarray<double>& x, int n, int sign, double *w){

    /* This sequential function multiplies a complex vector x
       of length n, stored as pairs of reals, componentwise
       by a complex vector w of length n, if sign=1, and
       by conjg(w), if sign=-1. The result overwrites x.
    */

    int j, j1;
    double wr, wi, xr, xi;

    for(j=0; j<2*n; j +=2){
        j1= j+1;
        wr= w[j];
        if (sign==1) {
            wi= w[j1];
        } else {
            wi= -w[j1];
        }
        xr= x[j];
        xi= x[j1];
        x[j]=  wr*xr - wi*xi;
        x[j1]= wi*xr + wr*xi;
    }

} /* end twiddle */

void twiddle_init(int n, double alpha, int *rho, double  *w){

    /* This sequential function initializes the weight table w
       to be used in twiddling with a complex vector of length n, 
       stored as pairs of reals.
       n=2^m, m >= 0.
       alpha is a real shift parameter.
       rho is the bit-reversal permutation of length n,
       which must have been initialized before calling this function.
       The output w is a table of n complex values, stored as pairs of reals,
          exp(-2*pi*i*rho(j)*alpha/n), 0 <= j < n.
    */

    int j;
    double theta;

    theta= -2.0 * M_PI * alpha / (double)n;
    for(j=0; j<n; j++){
        w[2*j]=   cos(rho[j]*theta);
        w[2*j+1]= sin(rho[j]*theta);
    }
    
} /* end twiddle_init */

void permute(bulk::coarray<double>& x, int n, int *sigma){

    /* This in-place sequential function permutes a complex vector x
       of length n >= 1, stored as pairs of reals, by the permutation sigma,
           y[j] = x[sigma[j]], 0 <= j < n.
       The output overwrites the vector x.
       sigma is a permutation of length n that must be decomposable
       into disjoint swaps.
    */

    int j, j0, j1, j2, j3;
    double tmpr, tmpi;

    for(j=0; j<n; j++){
        if (j<sigma[j]){
            /* swap components j and sigma[j] */
            j0= 2*j;
            j1= j0+1;
            j2= 2*sigma[j];
            j3= j2+1;
            tmpr= x[j0];
            tmpi= x[j1];
            x[j0]= x[j2];
            x[j1]= x[j3];
            x[j2]= tmpr;
            x[j3]= tmpi;
        }
    }
           
} /* end permute */

void bitrev_init(int n, int *rho){

    /* This function initializes the bit-reversal permutation rho 
       of length n, with n=2^m, m >= 0.
    */

    int j;
    unsigned int n1, rem, val, k, lastbit, one=1;

    if (n==1){
        rho[0]= 0;
        return;
    }
    n1= n;
    for(j=0; j<n; j++){
        rem= j; /* j= (b(m-1), ... ,b1,b0) in binary */
        val= 0;
        for (k=1; k<n1; k <<= 1){
            lastbit= rem & one; /* lastbit = b(i) with i= log2(k) */
            rem >>= 1;          /* rem = (b(m-1), ... , b(i+1)) */
            val <<= 1;
            val |= lastbit;     /* val = (b0, ... , b(i)) */
        }
        rho[j]= (int)val;
   }

} /* end bitrev_init */

/****************** Parallel functions ********************************/
int k1_init(int n, int p){
    
    /* This function computes the largest butterfly size k1 of the first
       superstep in a parallel FFT of length n on p processors with p < n.
    */
 
    int np, c, k1;

    np= n/p;
    for(c=1; c<p; c *=np)
        ;
    k1= n/c;

    return k1;

} /* end k1_init */

// x: coarray of size 2 * n in total, so 2n/p per core
void bspredistr(bulk::coarray<double>& xs, int n, int p, int s, int c0, int c1,
                char rev, int* rho_p) {

    /* This function redistributes the complex vector x of length n,
       stored as pairs of reals, from group-cyclic distribution
       over p processors with cycle c0 to cycle c1, where
       c0, c1, p, n are powers of two with 1 <= c0 <= c1 <= p <= n.
       s is the processor number, 0 <= s < p.
       If rev=true, the function assumes the processor numbering
       is bit reversed on input.
       rho_p is the bit-reversal permutation of length p.
    */

    int np, j0, j2, j, jglob, ratio, size, npackets, destproc, destindex, r;
 
    np= n/p;
    ratio= c1/c0;
    size= std::max(np/ratio,1);
    npackets= np/size;

    std::vector<double> tmp(2 * size);

    if (rev) {
        j0= rho_p[s]%c0; 
        j2= rho_p[s]/c0; 
    } else {
        j0= s%c0;
        j2= s/c0;
    }
    for(j=0; j<npackets; j++){
        jglob= j2*c0*np + j*c0 + j0;
        destproc=  (jglob/(c1*np))*c1 + jglob%c1;
        destindex= (jglob%(c1*np))/c1;
        for(r=0; r<size; r++){
            tmp[2*r]   = xs[2*(j+r*ratio)];
            tmp[2*r+1] = xs[2*(j+r*ratio)+1];
        }
        xs.put(destproc, {destindex * 2, (destindex + size) * 2}, tmp);
        //bsp_put(destproc,tmp,x,destindex*2*SZDBL,size*2*SZDBL);
    }
    xs.world().sync();

} /* end bspredistr */

void bspfft(bulk::coarray<double>& xs, int n, int p, int s, int sign, double *w0, double *w,
            double *tw, int *rho_np, int *rho_p){

    /* This parallel function computes the discrete Fourier transform
       of a complex array x of length n=2^m, m >= 1, stored in a real array
       of length 2n as pairs (Re x[j], Im x[j]), 0 <= j < n.
       x must have been registered before calling this function.
       p is the number of processors, p=2^q, 0 <= q < m.
       s is the processor number, 0 <= s < p.
       The function uses three weight tables:
           w0 for the unordered fft of length k1,
           w  for the unordered fft of length n/p,
           tw for a number of twiddles, each of length n/p.
       The function uses two bit-reversal permutations:
           rho_np of length n/p,
           rho_p of length p. 
       The weight tables and bit-reversal permutations must have been
       initialized before calling this function.
       If sign = 1, then the dft is computed,
           y[k] = sum j=0 to n-1 exp(-2*pi*i*k*j/n)*x[j], for 0 <= k < n.
       If sign =-1, then the inverse dft is computed,
           y[k] = (1/n) sum j=0 to n-1 exp(+2*pi*i*k*j/n)*x[j], for 0 <= k < n.
       Here, i=sqrt(-1). The output vector y overwrites x.
    */

    char rev;
    int np, k1, r, c0, c, ntw;
    double ninv;

    np= n/p;
    k1= k1_init(n,p);
    permute(xs,np,rho_np);
    rev= 1;
    for(r=0; r<np/k1; r++)
        ufft(xs.begin() + (2*r*k1),k1,sign,w0);

    c0= 1;
    ntw= 0;
    for (c=k1; c<=p; c *=np){   
        bspredistr(xs,n,p,s,c0,c,rev,rho_p);
        rev= 0;
        twiddle(xs,np,sign,&tw[2*ntw*np]);
        ufft(xs.begin(), np, sign, w);
        c0= c;
        ntw++;
    }

    if (sign==-1){
        ninv= 1 / (double)n;
        for (auto& x : xs)
            x *= ninv;
    }

} /* end bspfft */

void bspfft_init(int n, int p, int s, double *w0, double *w, double *tw,
                 int *rho_np, int *rho_p){

    /* This parallel function initializes all the tables used in the FFT. */

    int np, k1, ntw, c;
    double alpha;

    np= n/p;
    bitrev_init(np,rho_np);
    bitrev_init(p,rho_p);

    k1= k1_init(n,p);
    ufft_init(k1,w0);
    ufft_init(np,w);

    ntw= 0;
    for (c=k1; c<=p; c *=np){   
        alpha= (s%c) / (double)(c);
        twiddle_init(np,alpha,rho_np,&tw[2*ntw*np]);
        ntw++;
    }

} /* end bspfft_init */



/*  This is a test program which uses bspfft to perform 
    a Fast Fourier Transform and its inverse.

    The input vector is defined by x[j]=j+i, for 0 <= j < n.
    Here i= sqrt(-1).
 
    The output vector should equal the input vector,
    up to roundoff errors. Output is by triples (j, Re x[j], Im x[j]).
    Warning: don't rely on this test alone to check correctness. 
    (After all, deleting the main loop will give similar results ;) 

*/


#define NITERS 50  // Perform NITERS forward and backward transforms.
                   // A large NITERS helps to obtain accurate timings.
#define NPRINT 0   // Print NPRINT values per processor
#define MEGA 1000000.0

void bspfft_test(bulk::world& world, int n) {
    int s = world.rank();
    int p = world.active_processors();

    if (s == 0) {
        world.log("FFT of vector of length %d using %d processors", n, p);
        world.log("performing %d forward and %d backward transforms", NITERS,
                  NITERS);
    }

    int np = n / p;
    bulk::coarray<double> x(world, 2 * np);
    bulk::coarray<double> Error(world, p);

    int k1 = k1_init(n, p);
    std::vector<double> w0(k1);
    std::vector<double> w(np);
    std::vector<double> tw(2 * np + p);
    std::vector<int> rho_np(np);
    std::vector<int> rho_p(p);

    int q, jglob, it;
    double time0, time1, ffttime, nflops, max_error, error_re, error_im,
        error;


    // Initialize x
    for (int j = 0; j < np; j++) {
        jglob = j * p + s;
        x[2 * j] = (double)jglob;
        x[2 * j + 1] = 1.0;
    }

    world.sync();


    bulk::util::timer timer0;
    /* Initialize the weight and bit reversal tables */
    for (it = 0; it < NITERS; it++)
        bspfft_init(n, p, s, &w0[0], &w[0], &tw[0], &rho_np[0], &rho_p[0]);
    world.sync();
    time0 = timer0.get();

    /* Perform the FFTs */
    bulk::util::timer timer1;
    for (it = 0; it < NITERS; it++) {
        bspfft(x, n, p, s,  1, &w0[0], &w[0], &tw[0], &rho_np[0], &rho_p[0]);
        bspfft(x, n, p, s, -1, &w0[0], &w[0], &tw[0], &rho_np[0], &rho_p[0]);
    }
    world.sync();
    time1 = timer1.get();

    /* Compute the accuracy */
    max_error = 0.0;
    for (int j = 0; j < np; j++) {
        jglob = j * p + s;
        error_re = fabs(x[2 * j] - (double)jglob);
        error_im = fabs(x[2 * j + 1] - 1.0);
        error = sqrt(error_re * error_re + error_im * error_im);
        if (error > max_error)
            max_error = error;
    }
    Error(0)[s] = max_error;
    world.sync();

    if (s == 0) {
        max_error = 0.0;
        for (q = 0; q < p; q++) {
            if (Error[q] > max_error)
                max_error = Error[q];
        }
    }

    for (int j = 0; j < NPRINT && j < np; j++) {
        jglob = j * p + s;
        world.log("proc=%d j=%d Re= %f Im= %f", s, jglob, x[2 * j],
                  x[2 * j + 1]);
    }
    world.sync();

    if (s == 0) {
        world.log("Time per initialization = %lf sec", time0 / NITERS);
        ffttime = time1 / (2.0 * NITERS);
        world.log("Time per FFT = %lf sec", ffttime);
        nflops = 5 * n * log((double)n) / log(2.0) + 2 * n;
        world.log("Computing rate in FFT = %lf Mflop/s",
                  nflops / (MEGA * ffttime));
        world.log("Absolute error= %e", max_error);
        world.log("Relative error= %e\n", max_error / n);
    }
    world.sync();
}

