#include "polymatrix.h"
#include <stdexcept>

namespace fheprac
{
    PolyMatrix::PolyMatrix() : row_size_(0), col_size_(0), deg_(0), mod_(0) {}

    PolyMatrix::PolyMatrix(uint64_t row_size, uint64_t col_size, uint64_t degree, uint64_t modulus, uint64_t value)
        : row_size_(row_size), col_size_(col_size), deg_(degree), mod_(modulus)
    {
        elems_.assign(row_size_, std::vector<Polynomial>(col_size_));

        for (uint64_t r = 0; r < row_size_; r++)
        {
            for (uint64_t c = 0; c < col_size_; c++)
            {
                elems_[r][c].assign(deg_, mod_, value);
            }
        }
    }

    uint64_t PolyMatrix::get(uint64_t row, uint64_t col, uint64_t index) const
    {
        if (row >= row_size_ || col >= col_size_)
        {
            throw std::out_of_range("Row/Col out of range.");
        }

        return elems_[row][col].get(index);
    }

    Polynomial PolyMatrix::get(uint64_t row, uint64_t col) const
    {
        if (row >= row_size_ || col >= col_size_)
        {
            throw std::out_of_range("Row/Col out of range.");
        }

        return elems_[row][col];
    }

    void PolyMatrix::set(uint64_t row, uint64_t col, uint64_t index, uint64_t value)
    {
        if (row >= row_size_ || col >= col_size_)
        {
            throw std::out_of_range("Row/Col out of range.");
        }

        elems_[row][col].set(index, value);
    }

    void PolyMatrix::set(uint64_t row, uint64_t col, Polynomial poly)
    {
        for (uint64_t i = 0; i <= poly.degree(); i++)
        {
            set(row, col, i, poly.get(i));
        }
    }

    void PolyMatrix::assign(uint64_t row_size, uint64_t col_size, uint64_t degree, uint64_t modulus, uint64_t value)
    {
        row_size_ = row_size;
        col_size_ = col_size;
        deg_ = degree;
        mod_ = modulus;
        elems_.assign(row_size_, std::vector<Polynomial>(col_size_));

        for (uint64_t r = 0; r < row_size_; r++)
        {
            for (uint64_t c = 0; c < col_size_; c++)
            {
                elems_[r][c].assign(deg_, mod_, value);
            }
        }
    }

    uint64_t PolyMatrix::row_size() const
    {
        return row_size_;
    }

    uint64_t PolyMatrix::col_size() const
    {
        return col_size_;
    }

    uint64_t PolyMatrix::degree() const
    {
        return deg_;
    }

    uint64_t PolyMatrix::modulus() const
    {
        return mod_;
    }

    PolyMatrix PolyMatrix::t() const
    {
        PolyMatrix destination(col_size_, row_size_, deg_, mod_);

        for (uint64_t r = 0; r < row_size_; r++)
        {
            for (uint64_t c = 0; c < col_size_; c++)
            {
                destination.elems_[c][r] = elems_[r][c];
            }
        }

        return destination;
    }

    PolyMatrix PolyMatrix::operator+(const PolyMatrix& other) const
    {
        if (row_size_ != other.row_size_ || col_size_ != other.col_size_ || deg_ != other.deg_ || mod_ != other.mod_)
        {
            throw std::invalid_argument("PolyMatrixs must have matching dimension, degree and modulus.");
        }

        PolyMatrix destination(row_size_, col_size_, deg_, mod_);

        for (uint64_t r = 0; r < row_size_; r++)
        {
            for (uint64_t c = 0; c < col_size_; c++)
            {
                destination.elems_[r][c] = elems_[r][c] + other.elems_[r][c];
            }
        }

        return destination;
    }

    void PolyMatrix::operator+=(const PolyMatrix& other)
    {
        if (row_size_ != other.row_size_ || col_size_ != other.col_size_ || deg_ != other.deg_ || mod_ != other.mod_)
        {
            throw std::invalid_argument("PolyMatrixs must have matching dimension, degree and modulus.");
        }

        for (uint64_t r = 0; r < row_size_; r++)
        {
            for (uint64_t c = 0; c < col_size_; c++)
            {
                elems_[r][c] += other.elems_[r][c];
            }
        }
    }

    PolyMatrix PolyMatrix::operator-(const PolyMatrix& other) const
    {
        if (row_size_ != other.row_size_ || col_size_ != other.col_size_ || deg_ != other.deg_ || mod_ != other.mod_)
        {
            throw std::invalid_argument("PolyMatrixs must have matching dimension, degree and modulus.");
        }

        PolyMatrix destination(row_size_, col_size_, deg_, mod_);

        for (uint64_t r = 0; r < row_size_; r++)
        {
            for (uint64_t c = 0; c < col_size_; c++)
            {
                destination.elems_[r][c] = elems_[r][c] - other.elems_[r][c];
            }
        }

        return destination;
    }

    void PolyMatrix::operator-=(const PolyMatrix& other)
    {
        if (row_size_ != other.row_size_ || col_size_ != other.col_size_ || deg_ != other.deg_ || mod_ != other.mod_)
        {
            throw std::invalid_argument("PolyMatrixs must have matching dimension, degree and modulus.");
        }

        for (uint64_t r = 0; r < row_size_; r++)
        {
            for (uint64_t c = 0; c < col_size_; c++)
            {
                elems_[r][c] -= other.elems_[r][c];
            }
        }
    }

    PolyMatrix PolyMatrix::operator-() const
    {
        PolyMatrix destination(row_size_, col_size_, deg_, mod_);

        for (uint64_t r = 0; r < row_size_; r++)
        {
            for (uint64_t c = 0; c < col_size_; c++)
            {
                destination.elems_[r][c] = -(elems_[r][c]);
            }
        }

        return destination;
    }


    PolyMatrix PolyMatrix::operator*(const PolyMatrix& other) const
    {
        if (col_size_ != other.row_size_)
        {
            throw std::invalid_argument("PolyMatrixs must have matching dimension.");
        }

        if (deg_ != other.deg_ || mod_ != other.mod_)
        {
            throw std::invalid_argument("PolyMatrixs must have matching degree and modulus.");
        }

        PolyMatrix destination(row_size_, other.col_size_, deg_, mod_);

        for (uint64_t r = 0; r < row_size_; r++)
        {
            for (uint64_t c = 0; c < other.col_size_; c++)
            {
                for (uint64_t i = 0; i < col_size_; i++)
                {
                    destination.elems_[r][c] += (elems_[r][i] * other.elems_[i][c]);
                }
            }
        }

        return destination;
    }

    PolyMatrix PolyMatrix::operator*(const uint64_t& other) const
    {
        PolyMatrix destination(row_size_, col_size_, deg_, mod_);

        for (uint64_t r = 0; r < row_size_; r++)
        {
            for (uint64_t c = 0; c < col_size_; c++)
            {
                destination.elems_[r][c] = elems_[r][c] * other;
            }
        }

        return destination;
    }

    void PolyMatrix::operator*=(const PolyMatrix& other)
    {
        if (row_size_ != other.row_size_ || col_size_ != other.col_size_ || deg_ != other.deg_ || mod_ != other.mod_)
        {
            throw std::invalid_argument("PolyMatrixs must have matching dimension, degree and modulus.");
        }

        PolyMatrix destination(row_size_, other.col_size_, deg_, mod_);

        for (uint64_t r = 0; r < row_size_; r++)
        {
            for (uint64_t c = 0; c < other.col_size_; c++)
            {
                for (uint64_t i = 0; i < col_size_; i++)
                {
                    destination.elems_[r][c] += (elems_[r][i] * other.elems_[i][c]);
                }
            }
        }

        assign(row_size_, other.col_size_, deg_, mod_);

        for (uint64_t r = 0; r < row_size_; r++)
        {
            for (uint64_t c = 0; c < other.col_size_; c++)
            {
                elems_[r][c] = destination.elems_[r][c];
            }
        }
    }

    void PolyMatrix::operator*=(const uint64_t& other)
    {
        for (uint64_t r = 0; r < row_size_; r++)
        {
            for (uint64_t c = 0; c < col_size_; c++)
            {
                elems_[r][c] = elems_[r][c] * other;
            }
        }
    }
}