#ifndef TCP_STREAMBUF__H_99C656BC7F37__
#define TCP_STREAMBUF__H_99C656BC7F37__
#pragma once

#include <streambuf>
#include <memory>
#include <array>
#include <iostream>
#include "native.h"

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (static_cast<native::Socket>(-1))
#endif

template< class CharT, class Traits = std::char_traits<CharT>, size_t BufSize=256>
class basic_tcp_streambuf : public std::basic_streambuf<CharT,Traits>
{
    using super=std::basic_streambuf<CharT,Traits>;
public:
    using int_type=typename Traits::int_type;
    using char_type=typename Traits::char_type;
    using Socket=native::Socket;
    basic_tcp_streambuf();
    explicit
    basic_tcp_streambuf(Socket s);
    basic_tcp_streambuf(const basic_tcp_streambuf &)=delete;
    basic_tcp_streambuf(basic_tcp_streambuf &&)=default;
    basic_tcp_streambuf &operator=(const basic_tcp_streambuf &)=delete;
    basic_tcp_streambuf &operator=(basic_tcp_streambuf &&)=default;
    virtual ~basic_tcp_streambuf() = default;
protected:
    int sync()override;
    int_type underflow()override;
    int_type overflow( int_type ch )override;
private:
    int_type write_flush( );
private:
    static Socket                                       InvalidSocket;    
    std::array<char_type,BufSize>              write_buf{};
    std::array<char_type,BufSize>              read_buf{};
    Socket                                                  s_{INVALID_SOCKET};
//    std::future<void>                                write_ready{};
};

template< class CharT, class Traits, size_t BufSize >
basic_tcp_streambuf<CharT,Traits,BufSize>::basic_tcp_streambuf()
{
    super::setg( (CharT *)read_buf.data(), (CharT *)read_buf.data(), (CharT *)read_buf.data() + read_buf.size() + 1);
    super::setp( (CharT *)write_buf.data(), (CharT *)write_buf.data()+ write_buf.size() + 1 );
}

template< class CharT, class Traits, size_t BufSize >
basic_tcp_streambuf<CharT,Traits,BufSize>::basic_tcp_streambuf(native::Socket s)
: basic_tcp_streambuf()
{
    s_ = s;
}

template< class CharT, class Traits, size_t BufSize >
typename basic_tcp_streambuf<CharT,Traits,BufSize>::int_type 
basic_tcp_streambuf<CharT,Traits,BufSize>::write_flush( )
{
    auto *pBegin = super::pbase();
    auto *pCur = super::pptr();
    if (pCur > pBegin )
    {
        auto len = (pCur - pBegin+1);
        auto sent = ::send( s_, pBegin, (int)len, 0 );
       auto *pBegin = write_buf.data();
        auto *pEnd = pBegin + write_buf.size() + 1;
        if( sent > 0 )
        {
             super::setp( pBegin, pEnd );
            return Traits::not_eof(0);
        }
        else
        {
            super::setp( pEnd, pEnd );
            return Traits::eof();
        }
    }
    return 0;    
}

template< class CharT, class Traits, size_t BufSize >
typename basic_tcp_streambuf<CharT,Traits,BufSize>::int_type 
basic_tcp_streambuf<CharT,Traits,BufSize>::overflow( int_type ch )
{
    auto retVal = write_flush();
    if( Traits::eq_int_type(Traits::eof(), ch)  )
    {
        return Traits::not_eof(ch);
    }
    
    if ( Traits::eq_int_type(retVal, Traits::eof()) != true )
    {
        retVal = Traits::not_eof(ch);
        auto *ptr = write_buf.data();

        *ptr = Traits::to_char_type(ch);
        ++ptr;
        super::setp( ptr , write_buf.data() + write_buf.size() + 1 );
    }
    return retVal;
}

template< class CharT, class Traits, size_t BufSize >
typename basic_tcp_streambuf<CharT,Traits,BufSize>::int_type 
basic_tcp_streambuf<CharT,Traits,BufSize>::underflow()
{
    auto received = ::recv(s_, read_buf.data(), BufSize, 0);
    if( received > 0 )
    {
        super::setg( read_buf.data(), read_buf.data(), read_buf.data()+received );
        return Traits::to_int_type(read_buf.front());
    }
    auto *pBegin = read_buf.data();
    auto *pEnd = pBegin + read_buf.size() + 1;
    super::setg( pBegin, pEnd, pEnd);
    return Traits::eof();
}

template< class CharT, class Traits, size_t BufSize >
int
basic_tcp_streambuf<CharT,Traits,BufSize>::sync()
{
    write_flush();
    underflow();
    return super::sync();
}




#endif // TCP_STREAMBUF__H_99C656BC7F37__
