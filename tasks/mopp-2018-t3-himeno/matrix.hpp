template<typename T>
Matrix<T>::Matrix( uint nums, uint rows, uint cols, uint deps, uint num_threads ) :
    m_uiNums(nums),
    m_uiRows(rows),
    m_uiCols(cols),
    m_uiDeps(deps),
    m_pData(new T[nums * rows * cols * deps * sizeof(T)]),
    m_uiNumThreads(num_threads),
    m_pThreads(new std::thread[num_threads]),
    m_pWorking_ranges(new uint[num_threads+1]),
    m_uiNumMemoryOffset(rows * cols * deps),
    m_uiRowMemoryOffset(cols * deps)
{

    // calculate the working ranges for the threads
    for (uint i=0; i<m_uiNumThreads; i++) m_pWorking_ranges[i] = i * m_uiRows / m_uiNumThreads;
    m_pWorking_ranges[m_uiNumThreads] = m_uiRows;

}

template<typename T>
Matrix<T>::~Matrix() {
    if (m_pData != nullptr) delete[] m_pData;
    if (m_pThreads != nullptr) delete[] m_pThreads;
}

template<typename T>
void Matrix<T>::fill( T value, uint n, uint r ) {
    std::fill_n(&at(n, r, 0, 0), m_uiCols * m_uiDeps, value);
}

template<typename T>
void Matrix<T>::fill( T value, uint n ) {
    for (uint i = 0; i < m_uiNumThreads; i++) m_pThreads[i] = std::thread(std::fill_n<T* const, uint, T>, &at(n, m_pWorking_ranges[i], 0, 0), (m_pWorking_ranges[i+1]-m_pWorking_ranges[i]) * m_uiCols * m_uiDeps, value);
    for (uint i = 0; i < m_uiNumThreads; i++) m_pThreads[i].join();
}

template<typename T>
void Matrix<T>::fill( T value ) {
    for (uint n = 0; n < m_uiNums; n++) {
        for (uint i = 0; i < m_uiNumThreads; i++) m_pThreads[i] = std::thread(std::fill_n<T* const, uint, T>, &at(n, m_pWorking_ranges[i], 0, 0), (m_pWorking_ranges[i+1]-m_pWorking_ranges[i]) * m_uiCols * m_uiDeps, value);
        for (uint i = 0; i < m_uiNumThreads; i++) m_pThreads[i].join();
    }
}

template<typename T>
void Matrix<T>::fill_partial( T value, uint n_begin, uint n_end ) {
    for (uint n = n_begin; n < n_end; n++) {
        for (uint i = 0; i < m_uiNumThreads; i++) m_pThreads[i] = std::thread(std::fill_n<T* const, uint, T>, &at(n, m_pWorking_ranges[i], 0, 0), (m_pWorking_ranges[i+1]-m_pWorking_ranges[i]) * m_uiCols * m_uiDeps, value);
        for (uint i = 0; i < m_uiNumThreads; i++) m_pThreads[i].join();
    }
}

template<typename T>
void Matrix<T>::set_init_partial( Matrix<T> *m, uint r_begin, uint r_end ) {
    
    const T num_rows_squared = (T)((m->m_uiRows - 1)*(m->m_uiRows - 1));
    T value;

    for (uint n = 0; n < m->m_uiNums; n++) {
        for (uint r = r_begin; r < r_end; r++) {
            value = (T)(r*r) / num_rows_squared;
            m->fill(value, n, r);
        }
    }

}

template<typename T>
void Matrix<T>::set_init() {
    for (uint n = 0; n < m_uiNums; n++) {
        for (uint i = 0; i < m_uiNumThreads; i++) m_pThreads[i] = std::thread(Matrix<T>::set_init_partial, this, m_pWorking_ranges[i], m_pWorking_ranges[i+1]);
        for (uint i = 0; i < m_uiNumThreads; i++) m_pThreads[i].join();
    }
}

template<typename T>
T & Matrix<T>::at( uint n, uint r, uint c, uint d ) {
    return m_pData[n * m_uiNumMemoryOffset + r * m_uiRowMemoryOffset + c * m_uiDeps + d];
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

    const auto copy_size = (d_end-d_begin)*sizeof(T);

    const auto p_iterator_n = src->m_uiNumMemoryOffset - (r_end-r_begin)*src->m_uiRowMemoryOffset;
    const auto p_iterator_r = src->m_uiRowMemoryOffset - (c_end-c_begin)*src->m_uiDeps;
    const auto p_iterator_c = src->m_uiDeps;

    uint p_offset = n_begin*src->m_uiNumMemoryOffset + r_begin*src->m_uiRowMemoryOffset + c_begin*src->m_uiDeps + d_begin;

    for (uint n = n_begin; n < n_end; n++) {
        for (uint r = r_begin; r < r_end; r++) {
            for (uint c = c_begin; c < c_end; c++) {                
                std::memcpy(dst->m_pData+p_offset, src->m_pData+p_offset, copy_size);
                p_offset += p_iterator_c;
            }
            p_offset += p_iterator_r;
        }
        p_offset += p_iterator_n;
    }

}
