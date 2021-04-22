#ifndef __HEADER_MATRIX__
#define __HEADER_MATRIX__

#include "common.h"

class Matrix {

    public:

        const uint m_uiNums = 0;
        const uint m_uiRows = 0;
        const uint m_uiCols = 0;
        const uint m_uiDeps = 0;
        float* const m_pData = nullptr;

        Matrix() {}
        Matrix( uint nums, uint rows, uint cols, uint deps );
        ~Matrix();

        /**
         * @brief Fills the whole matrix at num n with the given
         * value
         * @param n The num/level to access
         * @param val The value to fill the matrix with
         */
        void set(int n, float val);

        void set_init();

        /**
         * @brief Access the matrix at the given position
         * @param num The num to access it at
         * @param row The row to access it at
         * @param col The column to access it at
         * @param depth The depth to access it at
         * @return A reference to the float at the given position
         */
        float & at( uint num, uint row, uint col, uint depth );

        /**
         * @brief Copies the contents of given src matrix into the dst matrix
         * @param src The source
         * @param dst The destination
         * @param n_begin The first num to copy
         * @param r_begin The first row to copy
         * @param c_begin The first column to copy
         * @param d_begin The first depth to copy
         * @param n_end The num to end copying (excluding)
         * @param r_end The row to end copying (excluding)
         * @param c_end The column to end copying (excluding)
         * @param d_end The depth to end copying (excluding)
         */
        static void copy( Matrix *src, Matrix *dst, uint n_begin, uint r_begin, uint c_begin, uint d_begin, uint n_end, uint r_end, uint c_end, uint d_end );

};

#endif