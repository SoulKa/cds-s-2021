#include "matrix.h"

#include <algorithm>

#include <stdio.h>

template<typename T>
Matrix<T>::Matrix( uint nums, uint rows, uint cols, uint deps )
    : m_uiNums(nums), m_uiRows(rows), m_uiCols(cols), m_uiDeps(deps), m_pData(new T[nums * rows * cols * deps * sizeof(T)]) {}

template<typename T>
Matrix<T>::~Matrix() {
    if (m_pData != nullptr) delete[] m_pData;
}

template<typename T>
void Matrix<T>::set( int n, T value ) {
    for (uint i=0; i<m_uiRows; i++) {
        for (uint j=0; j<m_uiCols; j++) {
            for (uint k=0; k<m_uiDeps; k++) at(n,i,j,k) = value;
        } 
    }
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
    return m_pData[(d) * m_uiCols * m_uiRows * m_uiNums + (c) * m_uiRows * m_uiNums + (r) * m_uiNums + (n)];
}

template<typename T>
void Matrix<T>::copy( Matrix<T> *src, Matrix<T> *dst, uint n_begin, uint r_begin, uint c_begin, uint d_begin, uint n_end, uint r_end, uint c_end, uint d_end ) {

    //fprintf(stderr, "Copying from [%u, %u, %u, %u] up to [%u, %u, %u, %u]\n", n_begin, r_begin, c_begin, d_begin, n_end, r_end, c_end, d_end);

    for (uint d = d_begin; d < d_end; d++) {
        for (uint c = c_begin; c < c_end; c++) {
            for (uint r = r_begin; r < r_end; r++) {
                for (uint n = n_begin; n < n_end; n++) dst->at(n, r, c, d) = src->at(n, r, c, d);
            }
        }
    }

}