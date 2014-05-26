//
//  UTF8StringUtils.cpp
//  Chilli Source
//  Created by Scott Downie on 26/05/2014.
//
//  The MIT License (MIT)
//
//  Copyright (c) 2014 Tag Games Limited
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//

#include <ChilliSource/Core/String/UTF8StringUtils.h>

namespace ChilliSource
{
    namespace Core
    {
        namespace UTF8StringUtils
        {
            //-----------------------------------------------------
            //-----------------------------------------------------
            std::string Append(UTF8Char in_char, const std::string& in_string)
            {
                std::string result(in_string);
                Append(in_char, result);
                return result;
            }
            //-----------------------------------------------------
            //-----------------------------------------------------
            void Append(UTF8Char in_char, std::string& out_appendedResult)
            {
                //Single byte code (ASCII)
                if(in_char <= 0x7F)
                {
                    out_appendedResult.reserve(1);
                    out_appendedResult +=((in_char>>0) & 0x7F);
                }
                //Two byte code
                else if(in_char <= 0x7FF)
                {
                    out_appendedResult.reserve(2);
                    out_appendedResult +=0xC0 | ((in_char>>6) & 0x1F);
                    out_appendedResult +=0x80 | ((in_char) & 0x3F);

                }
                //Three byte code
                else if(in_char <= 0xFFFF)
                {
                    out_appendedResult.reserve(3);
                    out_appendedResult +=0xE0 | ((in_char>>12) & 0x0F);
                    out_appendedResult +=0x80 | ((in_char>>6) & 0x3F);
                    out_appendedResult +=0x80 | ((in_char) & 0x3F);

                }
                //Four byte code
                else if(in_char <= 0x1FFFFF)
                {
                    out_appendedResult.reserve(4);
                    out_appendedResult +=0xF0 | ((in_char>>18) & 0x07);
                    out_appendedResult +=0x80 | ((in_char>>12) & 0x3F);
                    out_appendedResult +=0x80 | ((in_char>>6) & 0x3F);
                    out_appendedResult +=0x80 | ((in_char) & 0x3F);
                }
                //Invalid format
                else
                {
                    CS_LOG_FATAL("Invalid UT8 format");
                }
            }
        }
    }
}