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

        const uint m_uiNums = 0;
        const uint m_uiRows = 0;
        const uint m_uiCols = 0;
        const uint m_uiDeps = 0;
        T* const m_pData = nullptr;
        const uint m_uiNumThreads = 0;


        Matrix() {}
        Matrix( uint nums, uint rows, uint cols, uint deps, uint num_threads );
        ~Matrix();

        /**
         * @brief Fills the whole matrix at num n with the given
         * value
         * @param n The num/level to access
         * @param r The row to access
         * @param value The value to fill the matrix with
         */
        void fill(T value, uint n, uint r );
        void fill(T value, uint n );
        void fill(T value);

        void fill_partial( T value, uint n_begin, uint n_end );

        void set_init();

        /**
         * @brief Access the matrix at the given position
         * @param num The num to access it at
         * @param row The row to access it at
         * @param col The column to access it at
         * @param depth The depth to access it at
         * @return A reference to the float at the given position
         */
        T & at( uint num, uint row, uint col, uint depth );

        /**
         * @brief Copies the contents of given src matrix into the dst matrix
         * @param src The source matrix
         * @param dst The destination matrix
         * @param n_begin The first num to copy
         * @param r_begin The first row to copy
         * @param c_begin The first column to copy
         * @param d_begin The first depth to copy
         * @param n_end The num to end copying (excluding)
         * @param r_end The row to end copying (excluding)
         * @param c_end The column to end copying (excluding)
         * @param d_end The depth to end copying (excluding)
         */
        static void copy_partial( Matrix<T> *src, Matrix<T> *dst, uint n_begin, uint r_begin, uint c_begin, uint d_begin, uint n_end, uint r_end, uint c_end, uint d_end );
        static void copy( Matrix<T> *src, Matrix<T> *dst, uint n );
        static void copy( Matrix<T> *src, Matrix<T> *dst );

    private:
        std::thread* const m_pThreads = nullptr;
        uint* const m_pWorking_ranges = nullptr;

        const uint m_uiNumMemoryOffset = 0;
        const uint m_uiRowMemoryOffset = 0;

        static void set_init_partial( Matrix<T> *m, uint r_begin, uint r_end );

};

#include "matrix.hpp"

#endif