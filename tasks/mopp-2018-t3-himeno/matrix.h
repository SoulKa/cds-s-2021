#ifndef __HEADER_MATRIX__
#define __HEADER_MATRIX__

class Matrix {

    public:

        float* m_pData = nullptr;

        unsigned int m_uiNums = 0;

        unsigned int m_uiRows = 0;

        unsigned int m_uiCols = 0;

        unsigned int m_uiDeps = 0;

        ~Matrix();

        void initialize( unsigned int nums, unsigned int rows, unsigned int cols, unsigned int deps );

        void clear();

        void set(int l, float val);

        void set_init();

        float & at( unsigned int n, unsigned int r, unsigned int c, unsigned int d );

};

#endif