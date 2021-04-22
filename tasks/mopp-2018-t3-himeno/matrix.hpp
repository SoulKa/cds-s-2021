template<typename T>
Matrix<T>::Matrix( uint nums, uint rows, uint cols, uint deps )
    : m_uiNums(nums), m_uiRows(rows), m_uiCols(cols), m_uiDeps(deps), m_pData(new T[nums * rows * cols * deps * sizeof(T)]) {}

template<typename T>
Matrix<T>::~Matrix() {
    if (m_pData != nullptr) delete[] m_pData;
}

template<typename T>
void Matrix<T>::fill( T value ) {
    std::fill_n(m_pData, m_uiNums * m_uiRows * m_uiCols * m_uiDeps, value);
}

template<typename T>
void Matrix<T>::fill( T value, uint n ) {
    std::fill_n(&at(n, 0, 0, 0), m_uiRows * m_uiCols * m_uiDeps, value);
}

template<typename T>
void Matrix<T>::fill_partial( T value, uint n_begin, uint n_end ) {
    std::fill_n(&at(n_begin, 0, 0, 0), (n_end-n_begin) * m_uiRows * m_uiCols * m_uiDeps, value);
}

template<typename T>
void Matrix<T>::set_init() {
    for (uint i=0; i<m_uiRows; i++) {
        for (uint j=0; j<m_uiCols; j++) {
            for (uint k=0; k<m_uiDeps; k++) at(0,i,j,k) = (T)(i*i) / (T)((m_uiRows - 1)*(m_uiRows - 1));
        } 
    }
}

template<typename T>
T & Matrix<T>::at( uint n, uint r, uint c, uint d ) {
    return m_pData[(n) * m_uiRows * m_uiCols * m_uiDeps + (r) * m_uiCols * m_uiDeps + (c) * m_uiDeps + (d)];
}

template<typename T>
void Matrix<T>::copy( Matrix<T> *src, Matrix<T> *dst ) {
    assert((src->m_uiNums == dst->m_uiNums) && "The two matrices must have the same size when cloning!");
    std::memcpy(dst->m_pData, src->m_pData, (src->m_uiNums * src->m_uiRows * src->m_uiCols * src->m_uiDeps) * sizeof(T));
}

template<typename T>
void Matrix<T>::copy( Matrix<T> *src, Matrix<T> *dst, uint n ) {
    std::memcpy(&dst->at(n, 0, 0, 0), &src->at(n, 0, 0, 0), (src->m_uiRows * src->m_uiCols * src->m_uiDeps) * sizeof(T));
}

template<typename T>
void Matrix<T>::copy_partial( Matrix<T> *src, Matrix<T> *dst, uint n_begin, uint r_begin, uint c_begin, uint d_begin, uint n_end, uint r_end, uint c_end, uint d_end ) {
    for (uint n = n_begin; n < n_end; n++) {
        for (uint r = r_begin; r < r_end; r++) {
            for (uint c = c_begin; c < c_end; c++) {
                std::memcpy(&dst->at(n, r, c, d_begin), &src->at(n, r, c, d_begin), (d_end-d_begin)*sizeof(T));
            }
        }
    }
}
