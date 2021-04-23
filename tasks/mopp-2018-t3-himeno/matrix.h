#ifndef __HEADER_MATRIX__
#define __HEADER_MATRIX__

#include "common.h"

#include <cstring>
#include <algorithm>
#include <thread>

#include <stdio.h>
#include <assert.h>

template<typename T>
class Matrix {

    public:

        const uint m_uiRows = 0;
        const uint m_uiCols = 0;
        const uint m_uiDeps = 0;
        T* const m_pData = nullptr;
        const uint m_uiNumThreads = 0;


        Matrix() {}
        Matrix( uint rows, uint cols, uint deps, uint num_threads );
        ~Matrix();

        /**
         * @brief Fills the whole matrix at num n with the given
         * value
         * @param r The row to access
         * @param value The value to fill the matrix with
         */
        void fill(T value, uint r );
        void fill(T value);

        void set_init();

        /**
         * @brief Access the matrix at the given position
         * @param row The row to access it at
         * @param col The column to access it at
         * @param depth The depth to access it at
         * @return A reference to the float at the given position
         */
        T & at( uint row, uint col, uint depth );

        /**
         * @brief Copies the contents of given src matrix into the dst matrix
         * @param src The source matrix
         * @param dst The destination matrix
         * @param r_begin The first row to copy
         * @param c_begin The first column to copy
         * @param d_begin The first depth to copy
         * @param r_end The row to end copying (excluding)
         * @param c_end The column to end copying (excluding)
         * @param d_end The depth to end copying (excluding)
         */
        static void copy_partial( Matrix<T> *src, Matrix<T> *dst, uint r_begin, uint c_begin, uint d_begin, uint r_end, uint c_end, uint d_end );
        static void copy( Matrix<T> *src, Matrix<T> *dst );

    private:
        std::thread* const m_pThreads = nullptr;
        uint* const m_pWorking_ranges = nullptr;

        const uint m_uiRowMemoryOffset = 0;

        static void set_init_partial( Matrix<T> *m, uint r_begin, uint r_end );

};

#include "matrix.hpp"

#endif