/*
Copyright (c) 2019-2022 Alexander Scholz

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#ifndef YOURGAME_GLTEXTURE_H
#define YOURGAME_GLTEXTURE_H

#include <vector>
#include <utility> // pair
#include "yourgame/gl_include.h"

namespace yourgame
{
    namespace gl
    {
        class Texture
        {
        public:
            static Texture *make(GLenum target,
                                 GLenum unit,
                                 const std::vector<std::pair<GLenum, GLint>> &parameteri);

            /* deleting the copy constructor and the copy assignment operator
            prevents copying (and moving) of the object. */
            Texture(Texture const &) = delete;
            Texture &operator=(Texture const &) = delete;

            ~Texture();
            void bind() const;
            void unbindTarget() const;

            void updateData(GLenum target,
                            GLint level,
                            GLint internalformat,
                            GLsizei width,
                            GLsizei height,
                            GLint border,
                            GLenum format,
                            GLenum type,
                            const void *data,
                            bool generateMipmap);

            GLsizei width() const { return m_width; }
            GLsizei height() const { return m_height; }
            GLuint handle() const { return m_handle; }

        private:
            Texture() {}
            GLuint m_handle;
            GLenum m_target;
            GLenum m_unit;
            GLsizei m_width;
            GLsizei m_height;
        };
    }
} // namespace yourgame

#endif
