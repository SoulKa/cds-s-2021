#include "matrix.h"

Matrix::~Matrix() {
    clear();
}

void Matrix::initialize( unsigned int nums, unsigned int rows, unsigned int cols, unsigned int deps ) {
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
    for (unsigned int i=0; i<m_uiRows; i++) {
        for (unsigned int j=0; j<m_uiCols; j++) {
            for (unsigned int k=0; k<m_uiDeps; k++) at(l,i,j,k) = value;
        } 
    }
}

void Matrix::set_init() {
    for (unsigned int i=0; i<m_uiRows; i++) {
        for (unsigned int j=0; j<m_uiCols; j++) {
            for (unsigned int k=0; k<m_uiDeps; k++) at(0,i,j,k) = (float)(i*i) / (float)((m_uiRows - 1)*(m_uiRows - 1));
        } 
    }
}

float & Matrix::at( unsigned int n, unsigned int r, unsigned int c, unsigned int d ) {
    return m_pData[(n) * m_uiRows * m_uiCols * m_uiDeps + (r) * m_uiCols * m_uiDeps + (c) * m_uiDeps + (d)];
}