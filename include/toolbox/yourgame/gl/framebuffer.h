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
#ifndef YOURGAME_GLFRAMEBUFFER_H
#define YOURGAME_GLFRAMEBUFFER_H

#include <vector>
#include "yourgame/gl_include.h"
#include "yourgame/gl/texture.h"

namespace yourgame
{
    namespace gl
    {
        class Framebuffer
        {
        public:
            struct TextureAttachDescr
            {
                GLint internalformat;
                GLenum format;
                GLenum type;
                GLenum unit;
                std::vector<std::pair<GLenum, GLint>> parameteri;
                GLenum attachment;
            };

            static Framebuffer *make(GLsizei width,
                                     GLsizei height,
                                     std::vector<TextureAttachDescr> texAttachDescrs);
            ~Framebuffer();
            void bind();
            void unbindTarget();
            void resize(GLsizei width, GLsizei height);
            yourgame::gl::Texture *textureAttachment(int idx);

            /* deleting the copy constructor and the copy assignment operator
            prevents copying (and moving) of the object. */
            Framebuffer(Framebuffer const &) = delete;
            Framebuffer &operator=(Framebuffer const &) = delete;

        private:
            Framebuffer() {}
            GLuint m_handle;
            std::vector<yourgame::gl::Texture *> m_textureAttachments;
            std::vector<TextureAttachDescr> m_texAttachDescrs;
        };
    }
} // namespace yourgame

#endif
