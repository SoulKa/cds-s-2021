#include "matrix.h"

Matrix::Matrix( uint nums, uint rows, uint cols, uint deps )
    : m_uiNums(nums), m_uiRows(rows), m_uiCols(cols), m_uiDeps(deps), m_pData(new float[nums * rows * cols * deps * sizeof(float)]) {}

Matrix::~Matrix() {
    if (m_pData != nullptr) delete[] m_pData;
}

void Matrix::set( int n, float value ) {
    for (uint i=0; i<m_uiRows; i++) {
        for (uint j=0; j<m_uiCols; j++) {
            for (uint k=0; k<m_uiDeps; k++) at(n,i,j,k) = value;
        } 
    }
}

void Matrix::set_init() {
    for (uint i=0; i<m_uiRows; i++) {
        for (uint j=0; j<m_uiCols; j++) {
            for (uint k=0; k<m_uiDeps; k++) at(0,i,j,k) = (float)(i*i) / (float)((m_uiRows - 1)*(m_uiRows - 1));
        } 
    }
}

float & Matrix::at( uint n, uint r, uint c, uint d ) {
    return m_pData[(n) * m_uiRows * m_uiCols * m_uiDeps + (r) * m_uiCols * m_uiDeps + (c) * m_uiDeps + (d)];
}

void Matrix::copy( Matrix *src, Matrix *dst, uint n_begin, uint r_begin, uint c_begin, uint d_begin, uint n_end, uint r_end, uint c_end, uint d_end ) {

    for (; n_begin < n_end; n_begin++) {
        for (; r_begin < r_end; r_begin++) {
            for (; c_begin < c_end; c_begin++) {
                for (; c_begin < c_end; c_begin++) dst->at(n_begin, r_begin, c_begin, d_begin) = src->at(n_begin, r_begin, c_begin, d_begin);
            }
        }
    }

}