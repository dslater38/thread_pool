#include <vector>


class matrix
{
public:
    matrix(size_t rows, size_t cols )
    : rows_{rows}
    , cols_{cols}
	, array_( rows*cols, 0.0 )
    {
    }
    double &at(size_t r, size_t c)
    {
        return array_.at( r * cols_ + c ); 
    }
    
    matrix reduce( size_t col )
    {
        auto m = matrix{ rows_-1, cols_-1 };
        
        for( auto i=1; i<rows_; ++i )
        {
            auto jj =0;
            for( auto j=0; j<cols_; ++j )
            {
                if( j == col )
                    continue;
                else
                {
                    m.at(i-1,jj) = at(i,j);
                    ++jj;
                }
            }
        }
        return m;
    }
    
    double det()
    {
        if( rows_==2 && cols_==2 )
        {
            return at(0,0)*at(1,1) - at(0,1)*at(1,0);
        }
        else
        {
            auto sum = 0.0;
            auto a=-1.0;
            for( auto j=0; j<cols_; ++j )
            {
				a *= -1.0;
                sum += a*(at(0,j))*(reduce(j).det());
            }
            return sum;
        }
    }
    
private:
    size_t rows_{};
    size_t cols_{};
    std::vector<double> array_{};
};