/*
Copyright (c) 2019-2021 Alexander Scholz

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
#include <string>
#include <vector>
#include <map>
#include <array>
#include <exception>
#include "stb_image.h"
#include "tiny_obj_loader.h"
#include "nlohmann/json.hpp"
#include "yourgame/yourgame.h"
#include "yourgame/fileio.h"
#include "yourgame/glloading.h"
#include "yourgame/glconventions.h"

using json = nlohmann::json;

namespace yourgame
{
    GLTexture2D *loadTexture(const char *filename, GLenum unit, GLint minMaxFilter, bool generateMipmap)
    {
        return loadTexture(filename, unit, {{GL_TEXTURE_MIN_FILTER, minMaxFilter}, {GL_TEXTURE_MAG_FILTER, minMaxFilter}}, generateMipmap);
    }

    GLTextureAtlas *loadTextureAtlasCrunch(const char *filename, GLenum unit, GLint minMaxFilter, bool generateMipmap)
    {
        return loadTextureAtlasCrunch(filename, unit, {{GL_TEXTURE_MIN_FILTER, minMaxFilter}, {GL_TEXTURE_MAG_FILTER, minMaxFilter}}, generateMipmap);
    }

    GLTextureAtlas *loadTextureAtlasGrid(const char *filename, int tilesWidth, int tilesHeight, GLenum unit, GLint minMaxFilter, bool generateMipmap)
    {
        return loadTextureAtlasGrid(filename, tilesWidth, tilesHeight, unit, {{GL_TEXTURE_MIN_FILTER, minMaxFilter}, {GL_TEXTURE_MAG_FILTER, minMaxFilter}}, generateMipmap);
    }

    GLTexture2D *loadTexture(const char *filename, GLenum unit, std::vector<std::pair<GLenum, GLint>> parameteri, bool generateMipmap)
    {
        int width;
        int height;
        int numChannels;
        std::vector<uint8_t> imgData;
        yourgame::readAssetFile(filename, imgData);
        auto img = stbi_load_from_memory(imgData.data(), imgData.size(), &width, &height, &numChannels, 4);

        if (img)
        {
            yourgame::logd("loaded %v: %vx%vx%v", filename, width, height, numChannels);
            GLTexture2D *texture = GLTexture2D::make(GL_TEXTURE_2D, unit, parameteri);
            texture->updateData(GL_TEXTURE_2D,
                                0,
                                GL_RGBA8,
                                width,
                                height,
                                0,
                                GL_RGBA,
                                GL_UNSIGNED_BYTE,
                                img,
                                generateMipmap);
            stbi_image_free(img);
            return texture;
        }
        else
        {
            yourgame::logw("image %v failed to load", filename);
            return nullptr;
        }
    }

    GLTexture2D *loadCubemap(std::vector<std::string> filenames, GLenum unit, std::vector<std::pair<GLenum, GLint>> parameteri, bool generateMipmap)
    {
        GLTexture2D *texture = GLTexture2D::make(GL_TEXTURE_CUBE_MAP, unit, parameteri);

        for (int i = 0; i < filenames.size(); i++)
        {
            auto f = filenames[i];
            int width;
            int height;
            int numChannels;
            std::vector<uint8_t> imgData;
            yourgame::readAssetFile(f.c_str(), imgData);
            // assuming "flip vertically on load" is true, disable it here because
            // opengl expects cubemap textures "unflipped"...
            stbi_set_flip_vertically_on_load(false);
            auto img = stbi_load_from_memory(imgData.data(), imgData.size(), &width, &height, &numChannels, 4);
            // todo: we should check the state of "flip vertically on load" before and
            // restore it afterwards
            stbi_set_flip_vertically_on_load(true);

            if (img)
            {
                yourgame::logd("loaded %v: %vx%vx%v", f.c_str(), width, height, numChannels);
                texture->updateData(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                    0,
                                    GL_RGBA8,
                                    width,
                                    height,
                                    0,
                                    GL_RGBA,
                                    GL_UNSIGNED_BYTE,
                                    img,
                                    generateMipmap);
                stbi_image_free(img);
            }
            else
            {
                yourgame::logw("image %v failed to load", f.c_str());
                delete texture;
                return nullptr;
            }
        }

        return texture;
    }

    GLTextureAtlas *loadTextureAtlasCrunch(const char *filename, GLenum unit, std::vector<std::pair<GLenum, GLint>> parameteri, bool generateMipmap)
    {
        std::vector<uint8_t> atlasFile;
        if (yourgame::readAssetFile(filename, atlasFile))
        {
            yourgame::loge("failed to load %v", filename);
            return nullptr;
        }

        json jAtlas;
        try
        {
            jAtlas = json::parse(atlasFile);
        }
        catch (std::exception &e)
        {
            yourgame::loge("failed to parse json from %v: %v", filename, e.what());
            return nullptr;
        }

        GLTextureAtlas *newAtlas = new GLTextureAtlas();

        for (auto &jTex : jAtlas["textures"])
        {
            // todo: always assume .png for crunch atlasses?
            std::string texFileName = jTex["name"].get<std::string>() + ".png";

            auto newTex = yourgame::loadTexture(texFileName.c_str(), unit, parameteri, generateMipmap);
            if (newTex)
            {
                newAtlas->pushTexture(newTex);

                for (auto &jImg : jTex["images"])
                {
                    newAtlas->pushCoords(jImg["n"].get<std::string>(),
                                         (float)(jImg["x"].get<double>() / (double)newTex->width()),
                                         (float)((jImg["x"].get<double>() + jImg["w"].get<double>()) / (double)newTex->width()),
                                         (float)(1.0 - ((jImg["y"].get<double>() + jImg["h"].get<double>()) / (double)newTex->height())),
                                         (float)(1.0 - (jImg["y"].get<double>() / (double)newTex->height())));
                }
            }
            else
            {
                yourgame::loge("failed to load texture from file %v referenced in atlas %v", texFileName, filename);
                delete newAtlas;
                return nullptr;
            }
        }

        return newAtlas;
    }

    GLTextureAtlas *loadTextureAtlasGrid(const char *filename,
                                         int tilesWidth,
                                         int tilesHeight,
                                         GLenum unit,
                                         std::vector<std::pair<GLenum, GLint>> parameteri,
                                         bool generateMipmap)
    {
        auto newTex = yourgame::loadTexture(filename, unit, parameteri, generateMipmap);
        if (!newTex)
        {
            return nullptr;
        }

        GLTextureAtlas *newAtlas = new GLTextureAtlas();
        newAtlas->pushTexture(newTex);

        for (int h = 0; h < tilesHeight; h++)
        {
            for (int w = 0; w < tilesWidth; w++)
            {
                newAtlas->pushCoords(std::to_string(h * tilesWidth + w),
                                     ((float)w) / ((float)tilesWidth),
                                     ((float)(w + 1)) / ((float)tilesWidth),
                                     1.0f - (((float)(h + 1)) / ((float)tilesHeight)),
                                     1.0f - (((float)h) / ((float)tilesHeight)));
            }
        }

        return newAtlas;
    }

    GLShader *loadShader(std::vector<std::pair<GLenum, std::string>> shaderFilenames,
                         std::vector<std::pair<GLuint, std::string>> attrLocs,
                         std::vector<std::pair<GLuint, std::string>> fragDataLocs)
    {
        std::vector<std::pair<GLenum, std::string>> shaderCodes;

        for (const auto &shdrFile : shaderFilenames)
        {
            std::vector<uint8_t> shdrCode;
            yourgame::readAssetFile(shdrFile.second.c_str(), shdrCode);
            std::string shdrStr = std::string(YOURGAME_GLSL_VERSION_STRING) + "\n" +
                                  std::string(shdrCode.begin(), shdrCode.end());
            shaderCodes.push_back(std::make_pair(shdrFile.first, shdrStr));
        }

        std::string shaderErrLog;
        GLShader *newShader = GLShader::make(shaderCodes, attrLocs, fragDataLocs, shaderErrLog);

        if (!newShader)
        {
            yourgame::loge("shader failed to make: %v", shaderErrLog);
        }
        else
        {
            // set common uniform values
            newShader->useProgram();
            GLint unif;

            // diffuse texture
            unif = newShader->getUniformLocation(yourgame::unifNameTextureDiffuse);
            if (unif != -1)
                glUniform1i(unif, yourgame::unifValueTextureDiffuse);

            // sky texture
            unif = newShader->getUniformLocation(yourgame::unifNameTextureSky);
            if (unif != -1)
                glUniform1i(unif, yourgame::unifValueTextureSky);

            glUseProgram(0);
        }

        return newShader;
    }

    GLGeometry *loadGeometry(const char *objFilename, const char *mtlFilename)
    {
        yourgame::logd("loading geometry %v...", objFilename);

        std::vector<uint8_t> objData;
        if (yourgame::readAssetFile(objFilename, objData))
        {
            yourgame::loge("loadGeometry(): failed to load obj file %v", objFilename);
            return nullptr;
        }
        std::string objStr(objData.begin(), objData.end());

        std::string mtlStr = "";
        if (mtlFilename)
        {
            std::vector<uint8_t> mtlData;
            if (yourgame::readAssetFile(mtlFilename, mtlData))
            {
                yourgame::loge("loadGeometry(): failed to load mtl file %v", mtlFilename);
                return nullptr;
            }
            mtlStr = std::string(mtlData.begin(), mtlData.end());
        }

        tinyobj::ObjReader objRdr;
        tinyobj::ObjReaderConfig objRdrCfg;
        objRdrCfg.triangulate = true;
        objRdrCfg.vertex_color = true;
        if (!objRdr.ParseFromString(objStr, mtlStr, objRdrCfg))
        {
            yourgame::loge("tinyobj::ObjReader::ParseFromString failed");
            return nullptr;
        }

        auto shapes = objRdr.GetShapes();
        auto attribs = objRdr.GetAttrib();
        auto materials = objRdr.GetMaterials();

        yourgame::logd("%v shape(s)", shapes.size());
        yourgame::logd("%v material(s)", materials.size());

        std::vector<GLuint> objIdxData;
        std::vector<GLuint> objLineIdxData;
        std::vector<GLfloat> objPosData;
        std::vector<GLfloat> objNormalData;
        std::vector<GLfloat> objTexCoordData;
        std::vector<GLfloat> objColordData;

        // each mesh index (here: 3 of them form a triangle (face)), references:
        //   (1) vertex position (and color),
        //   (2) normal,
        //   (3) texture coordinates and
        //   (4) meterial index (of the face),
        // that are parsed separately.
        // uniqueIdxMap is used to track unique combinations of (1-4)
        std::map<std::array<int, 4>, int> uniqueIdxMap;
        GLuint uniqueVertCount = 0U;

        // we merge all obj meshes (triangle shapes) into one GLGeometry shape,
        // and all obj line shapes into another GLGeometry shape
        for (auto const &shape : shapes)
        {
            yourgame::logd("shape: %v", shape.name);

            GLuint shapeMeshReadIdx = 0U;
            for (auto const &idx : shape.mesh.indices)
            {
                // assuming all faces are triangles (objRdrCfg.triangulate is set above),
                // get the material id for that face (-1, if no material is assigned)
                int materialId = (shape.mesh.material_ids.at(shapeMeshReadIdx / 3U));
                auto mapRet = uniqueIdxMap.emplace(
                    std::array<int, 4>{idx.vertex_index, idx.normal_index, idx.texcoord_index, materialId}, uniqueVertCount);
                if (mapRet.second) // new unique vertex
                {
                    try
                    {
                        objIdxData.push_back(uniqueVertCount);
                        objPosData.push_back((GLfloat)attribs.vertices.at(idx.vertex_index * 3));
                        objPosData.push_back((GLfloat)attribs.vertices.at(idx.vertex_index * 3 + 1));
                        objPosData.push_back((GLfloat)attribs.vertices.at(idx.vertex_index * 3 + 2));

                        // if material available, use diffuse color as vertex color. if not, use the
                        // (always available, but maybe default) parsed vertex color
                        if (materialId > -1 && materialId < materials.size())
                        {
                            objColordData.push_back((GLfloat)materials.at(materialId).diffuse[0]);
                            objColordData.push_back((GLfloat)materials.at(materialId).diffuse[1]);
                            objColordData.push_back((GLfloat)materials.at(materialId).diffuse[2]);
                            // todo: optionally, make geometry buffers for other material components:
                            // ambient, specular, emissive colors, specular exponent ... ?
                        }
                        else
                        {
                            objColordData.push_back((GLfloat)attribs.colors.at(idx.vertex_index * 3));
                            objColordData.push_back((GLfloat)attribs.colors.at(idx.vertex_index * 3 + 1));
                            objColordData.push_back((GLfloat)attribs.colors.at(idx.vertex_index * 3 + 2));
                        }

                        // if normals available, use them
                        if (attribs.normals.size() > 0)
                        {
                            objNormalData.push_back((GLfloat)attribs.normals.at(idx.normal_index * 3));
                            objNormalData.push_back((GLfloat)attribs.normals.at(idx.normal_index * 3 + 1));
                            objNormalData.push_back((GLfloat)attribs.normals.at(idx.normal_index * 3 + 2));
                        }

                        // if texture coordinates available, use them
                        if (attribs.texcoords.size() > 0)
                        {
                            objTexCoordData.push_back((GLfloat)attribs.texcoords.at(idx.texcoord_index * 2));
                            objTexCoordData.push_back((GLfloat)attribs.texcoords.at(idx.texcoord_index * 2 + 1));
                        }
                    }
                    catch (...)
                    {
                        yourgame::loge("loadGeometry(): %v has faulty obj vertex data", objFilename);
                        return nullptr;
                    }
                    uniqueVertCount++;
                }
                else // reuse unique vertex index
                {
                    objIdxData.push_back((GLuint)(mapRet.first->second));
                }
                shapeMeshReadIdx++;
            }

            // lines. todo: work in progress.
            // - new unique vertices (for lines) are created with material = -1.
            //   (lines do not have material)
            // - as the line vertices share buffers with triangle face vertices,
            //   they must also reference texcoords/normals (if triangle faces do),
            //   in order to keep the vertex data "aligned"
            // - assumption: vertices per line: always 2
            for (auto const &idx : shape.lines.indices)
            {
                int materialId = -1;
                auto mapRet = uniqueIdxMap.emplace(
                    std::array<int, 4>{idx.vertex_index, idx.normal_index, idx.texcoord_index, materialId}, uniqueVertCount);
                if (mapRet.second) // new unique vertex
                {
                    try
                    {
                        objLineIdxData.push_back(uniqueVertCount);
                        objPosData.push_back((GLfloat)attribs.vertices.at(idx.vertex_index * 3));
                        objPosData.push_back((GLfloat)attribs.vertices.at(idx.vertex_index * 3 + 1));
                        objPosData.push_back((GLfloat)attribs.vertices.at(idx.vertex_index * 3 + 2));
                        objColordData.push_back((GLfloat)attribs.colors.at(idx.vertex_index * 3));
                        objColordData.push_back((GLfloat)attribs.colors.at(idx.vertex_index * 3 + 1));
                        objColordData.push_back((GLfloat)attribs.colors.at(idx.vertex_index * 3 + 2));

                        // if normals available, use them
                        if (attribs.normals.size() > 0)
                        {
                            objNormalData.push_back((GLfloat)attribs.normals.at(idx.normal_index * 3));
                            objNormalData.push_back((GLfloat)attribs.normals.at(idx.normal_index * 3 + 1));
                            objNormalData.push_back((GLfloat)attribs.normals.at(idx.normal_index * 3 + 2));
                        }

                        // if texture coordinates available, use them
                        if (attribs.texcoords.size() > 0)
                        {
                            objTexCoordData.push_back((GLfloat)attribs.texcoords.at(idx.texcoord_index * 2));
                            objTexCoordData.push_back((GLfloat)attribs.texcoords.at(idx.texcoord_index * 2 + 1));
                        }
                    }
                    catch (...)
                    {
                        yourgame::loge("loadGeometry(): %v has faulty line obj vertex data for", objFilename);
                        return nullptr;
                    }
                    uniqueVertCount++;
                }
                else // reuse unique vertex index
                {
                    objLineIdxData.push_back((GLuint)(mapRet.first->second));
                }
            }
        }

        GLGeometry *newGeo = GLGeometry::make();

        auto vertPosSize = objPosData.size() * sizeof(objPosData[0]);
        auto vertNormSize = objNormalData.size() * sizeof(objNormalData[0]);
        auto vertTexcoordsSize = objTexCoordData.size() * sizeof(objTexCoordData[0]);
        auto vertColorSize = objColordData.size() * sizeof(objColordData[0]);
        auto vertIdxSize = objIdxData.size() * sizeof(objIdxData[0]);
        auto vertLineIdxSize = objLineIdxData.size() * sizeof(objLineIdxData[0]);

        // add available buffers to new Geometry and configure
        // the Geometry shape...
        std::vector<GLShape::ArrBufferDescr> arDescrs =
            {{attrLocPosition, 3, GL_FLOAT, GL_FALSE, 0, (void *)0, 0},
             {attrLocColor, 3, GL_FLOAT, GL_FALSE, 0, (void *)0, 0}};

        std::vector<std::string> arBufferNames = {"pos", "color"};

        newGeo->addBuffer("pos", GL_ARRAY_BUFFER, vertPosSize, objPosData.data(), GL_STATIC_DRAW);
        newGeo->addBuffer("color", GL_ARRAY_BUFFER, vertColorSize, objColordData.data(), GL_STATIC_DRAW);
        newGeo->addBuffer("idx", GL_ELEMENT_ARRAY_BUFFER, vertIdxSize, objIdxData.data(), GL_STATIC_DRAW);
        newGeo->addBuffer("idxLines", GL_ELEMENT_ARRAY_BUFFER, vertLineIdxSize, objLineIdxData.data(), GL_STATIC_DRAW);
        if (vertNormSize > 0)
        {
            newGeo->addBuffer("norm", GL_ARRAY_BUFFER, vertNormSize, objNormalData.data(), GL_STATIC_DRAW);
            arDescrs.push_back({attrLocNormal, 3, GL_FLOAT, GL_FALSE, 0, (void *)0, 0});
            arBufferNames.push_back("norm");
        }
        if (vertTexcoordsSize > 0)
        {
            newGeo->addBuffer("texcoords", GL_ARRAY_BUFFER, vertTexcoordsSize, objTexCoordData.data(), GL_STATIC_DRAW);
            arDescrs.push_back({attrLocTexcoords, 2, GL_FLOAT, GL_FALSE, 0, (void *)0, 0});
            arBufferNames.push_back("texcoords");
        }

        // all obj triangle shapes
        newGeo->addShape("main", arDescrs, arBufferNames, {GL_UNSIGNED_INT, GL_TRIANGLES, (GLsizei)objIdxData.size()}, "idx");

        // all obj line shapes
        newGeo->addShape("lines", arDescrs, arBufferNames, {GL_UNSIGNED_INT, GL_LINES, (GLsizei)objLineIdxData.size()}, "idxLines");

        return newGeo;
    }
} // namespace yourgame
