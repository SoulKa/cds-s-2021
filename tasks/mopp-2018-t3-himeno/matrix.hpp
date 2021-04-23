template<typename T>
Matrix<T>::Matrix( int rows, int cols, int deps, uint num_threads ) :
    m_uiRows(rows),
    m_uiCols(cols),
    m_uiDeps(deps),
    m_pData(new T[rows*cols*deps]),
    m_uiDataSize(rows*cols*deps),
    m_uiRowMemoryOffset(cols * deps),
    m_uiNumThreads(num_threads),
    m_pThreads(new std::thread[num_threads]),
    m_pWorking_ranges(new int[num_threads+1]),
    m_uiRowsSquared((rows+1)*(rows+1))
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
void Matrix<T>::set_init_partial( Matrix<T> *m, int r_begin, int r_end ) {
    
    T value;
    for (int r = r_begin; r < r_end; r++) {
        value = (T)((r+1)*(r+1)) / (T)((m->m_uiRows+1)*(m->m_uiRows+1));
        std::fill_n(&m->at(r, 0, 0), m->m_uiRowMemoryOffset, value);
    }

}

template<typename T>
void Matrix<T>::set_init() {
    for (uint i = 0; i < m_uiNumThreads; i++) m_pThreads[i] = std::thread(Matrix<T>::set_init_partial, this, m_pWorking_ranges[i], m_pWorking_ranges[i+1]);
    for (uint i = 0; i < m_uiNumThreads; i++) m_pThreads[i].join();
}

template<typename T>
T & Matrix<T>::at( int r, int c, int d ) {
    return m_pData[r * m_uiRowMemoryOffset + c * m_uiDeps + d];
}

template<typename T>
T Matrix<T>::get( int r, int c, int d ) {
    if (r == -1) return 0.0;
    if (r == m_uiRows) return 1.0;
    if (c == -1 || d == -1 || c == m_uiCols || d == m_uiDeps) return (T)((r+1)*(r+1)) / (T)((m_uiRows+1)*(m_uiRows+1));
    return m_pData[r * m_uiRowMemoryOffset + c * m_uiDeps + d];
}

template<typename T>
void Matrix<T>::copy( Matrix<T> *src, Matrix<T> *dst ) {
    std::memcpy(dst->m_pData, src->m_pData, (src->m_uiRows * src->m_uiCols * src->m_uiDeps) * sizeof(T));
}
