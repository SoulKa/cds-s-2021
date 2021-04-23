template<typename T>
Matrix<T>::Matrix( uint rows, uint cols, uint deps, uint num_threads ) :
    m_uiRows(rows),
    m_uiCols(cols),
    m_uiDeps(deps),
    m_pData(new T[rows * cols * deps * sizeof(T)]),
    m_uiRowMemoryOffset(cols * deps),
    m_uiNumThreads(num_threads),
    m_pThreads(new std::thread[num_threads]),
    m_pWorking_ranges(new uint[num_threads+1])
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
void Matrix<T>::set_init_partial( Matrix<T> *m, uint r_begin, uint r_end ) {
    
    const T num_rows_squared = (T)((m->m_uiRows - 1)*(m->m_uiRows - 1));
    T value;

    for (uint r = r_begin; r < r_end; r++) {
        value = (T)(r*r) / num_rows_squared;
        for (uint c = 0; c < m->m_uiCols; c++) {
            for (uint d = 0; d < m->m_uiDeps; d++) m->at(r, c, d) = value;
        }
    }

}

template<typename T>
void Matrix<T>::set_init() {
    for (uint i = 0; i < m_uiNumThreads; i++) m_pThreads[i] = std::thread(Matrix<T>::set_init_partial, this, m_pWorking_ranges[i], m_pWorking_ranges[i+1]);
    for (uint i = 0; i < m_uiNumThreads; i++) m_pThreads[i].join();
}

template<typename T>
T & Matrix<T>::at( uint r, uint c, uint d ) {
    //return m_pData[r * m_uiRowMemoryOffset + c * m_uiDeps + d];
    return m_pData[r * m_uiRowMemoryOffset + c * m_uiDeps + d];
}

template<typename T>
void Matrix<T>::copy( Matrix<T> *src, Matrix<T> *dst ) {
    std::memcpy(dst->m_pData, src->m_pData, (src->m_uiRows * src->m_uiDeps * src->m_uiDeps) * sizeof(T));
}
