namespace bulk_base {

template <typename T>
class coarray {
  public:
    coarray(int local_size_);
    coarray_image<T>& operator[](int t) { };
}

template <typename T>
class coarray_image {
  public:
    coarray_writer<T>& operator[](int i) { };
}

template <typename T>
class coarray_writer {
  public:
    void operator=(T value);
}

} // namespace bulk_base
