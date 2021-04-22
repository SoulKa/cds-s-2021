#include "matrix.h"

Matrix::~Matrix() {
    clear();
}

void Matrix::initialize( uint nums, uint rows, uint cols, uint deps ) {
    m_uiNums = nums;
    m_uiRows = rows;
    m_uiCols = cols;
    m_uiDeps = deps;
    m_pData = new float[nums * rows * cols * deps * sizeof(float)];
}

void Matrix::clear() {

    if (m_pData == nullptr) return;

    delete[] m_pData;
    m_uiNums = 0;
    m_uiRows = 0;
    m_uiCols = 0;
    m_uiDeps = 0;

}

void Matrix::set( int l, float value ) {
    for (uint i=0; i<m_uiRows; i++) {
        for (uint j=0; j<m_uiCols; j++) {
            for (uint k=0; k<m_uiDeps; k++) at(l,i,j,k) = value;
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

void copy( Matrix *src, Matrix *dst, uint n_begin, uint r_begin, uint c_begin, uint d_begin, uint n_end, uint r_end, uint c_end, uint d_end ) {

}