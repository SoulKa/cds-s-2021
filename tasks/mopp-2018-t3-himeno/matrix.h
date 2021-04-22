#ifndef __HEADER_MATRIX__
#define __HEADER_MATRIX__

#include "common.h"

class Matrix {

    public:

        float* m_pData = nullptr;

        uint m_uiNums = 0;

        uint m_uiRows = 0;

        uint m_uiCols = 0;

        uint m_uiDeps = 0;

        ~Matrix();

        void initialize( uint nums, uint rows, uint cols, uint deps );

        void clear();

        void set(int l, float val);

        void set_init();

        float & at( uint num, uint row, uint col, uint depth );

};

#endif