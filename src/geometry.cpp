#include "geometry.h"

template <> template <> Vec3<int>::Vec3(const Vec3<float>& v) : x(static_cast<int>(v.x+.5)),
                                                                y(static_cast<int>(v.y+.5)),
                                                                z(static_cast<int>(v.z+.5)) {
}

template <> template <> Vec3<float>::Vec3(const Vec3<int>& v) : x(v.x), y(v.y), z(v.z) {
}

Matrix::Matrix(int row, int col) : mCols(col), mRows(row),
                                    mMatrix(std::vector<std::vector<float>>(row, std::vector<float>(col, 0.f))) {
}

Matrix Matrix::eye(int size) {
    Matrix mat{ size, size };
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            mat[i][j] = (i == j) ? 1 : 0;
        }
    }
    return mat;
}

std::vector<float>& Matrix::operator[](const int i) {
    assert(i >= 0 && i < mRows);
    return mMatrix[i];
}

Matrix Matrix::operator*(const Matrix& m) const {
    assert(mCols == m.mRows);
    Matrix res{ mRows, m.mCols };
    for (int i = 0; i < mRows; ++i) {
        for (int j = 0; j < m.mCols; ++j) {
            for (int k = 0; k < mCols; ++k) {
                res[i][j] += mMatrix[i][k] * m.mMatrix[k][j];
            }
        }
    }
    return res;
}

Matrix Matrix::transpose() {
    Matrix res{ mCols, mRows };
    for (int i = 0; i < mRows; ++i) {
        for (int j = 0; j < mCols; ++j) {
            res[j][i] = mMatrix[i][j];
        }
    }
    return res;
}

std::ostream& operator<<(std::ostream &s, const Matrix& m) {
    for (int i = 0; i < m.nrows(); ++i) {
        s << "| ";
        for (int j = 0; j < m.ncols(); ++j) {
            s << m.mMatrix[i][j] << ' ';
        }
        s << "|\n";
    }
    return s;
}
