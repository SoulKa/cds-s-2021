#define FILL_MUTLITHREADED true

template<typename T>
Matrix<T>::Matrix( uint rows, uint cols, uint deps, uint num_threads ) :
    m_uiRows(rows),
    m_uiCols(cols),
    m_uiDeps(deps),
    m_pData(new T[rows * cols * deps * sizeof(T)]),
    m_uiNumThreads(num_threads),
    m_pThreads(new std::thread[num_threads]),
    m_pWorking_ranges(new uint[num_threads+1]),
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
void Matrix<T>::fill( T value, uint r ) {
    std::fill_n(&at(r, 0, 0), m_uiRowMemoryOffset, value);
}

template<typename T>
void Matrix<T>::fill( T value ) {
    #if FILL_MUTLITHREADED
        for (uint i = 0; i < m_uiNumThreads; i++) m_pThreads[i] = std::thread(std::fill_n<T* const, uint, T>, &at(m_pWorking_ranges[i], 0, 0), (m_pWorking_ranges[i+1]-m_pWorking_ranges[i]) * m_uiCols * m_uiDeps, value);
        for (uint i = 0; i < m_uiNumThreads; i++) m_pThreads[i].join();
    #else
        std::fill_n(m_pData, m_uiNumMemoryOffset*m_uiNums, value);
    #endif
}

template<typename T>
void Matrix<T>::set_init_partial( Matrix<T> *m, uint r_begin, uint r_end ) {
    
    const T num_rows_squared = (T)((m->m_uiRows - 1)*(m->m_uiRows - 1));
    T value;

    for (uint r = r_begin; r < r_end; r++) {
        value = (T)(r*r) / num_rows_squared;
        m->fill(value, r);
    }

}

template<typename T>
void Matrix<T>::set_init() {
    for (uint i = 0; i < m_uiNumThreads; i++) m_pThreads[i] = std::thread(Matrix<T>::set_init_partial, this, m_pWorking_ranges[i], m_pWorking_ranges[i+1]);
    for (uint i = 0; i < m_uiNumThreads; i++) m_pThreads[i].join();
}

template<typename T>
T & Matrix<T>::at( uint r, uint c, uint d ) {
    return m_pData[r * m_uiRowMemoryOffset + c * m_uiDeps + d];
}

template<typename T>
void Matrix<T>::copy( Matrix<T> *src, Matrix<T> *dst ) {
    std::memcpy(dst->m_pData, src->m_pData, (src->m_uiRows * src->m_uiRowMemoryOffset) * sizeof(T));
}

template<typename T>
void Matrix<T>::copy_partial( Matrix<T> *src, Matrix<T> *dst, uint r_begin, uint c_begin, uint d_begin, uint r_end, uint c_end, uint d_end ) {

    const auto copy_size = (d_end-d_begin)*sizeof(T);

    const auto p_iterator_r = src->m_uiRowMemoryOffset - (c_end-c_begin)*src->m_uiDeps;
    const auto p_iterator_c = src->m_uiDeps;

    uint p_offset = r_begin*src->m_uiRowMemoryOffset + c_begin*src->m_uiDeps + d_begin;

    for (uint r = r_begin; r < r_end; r++) {
        for (uint c = c_begin; c < c_end; c++) {                
            std::memcpy(dst->m_pData+p_offset, src->m_pData+p_offset, copy_size);
            p_offset += p_iterator_c;
        }
        p_offset += p_iterator_r;
    }

}
