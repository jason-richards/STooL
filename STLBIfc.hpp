#pragma once

#include <memory>
#include <string>
#include <iostream>

#include "GraphSTL.hpp"


#pragma pack(push, 1)
    typedef struct
    STLFacet {
        float       m_Normal[3];
        float       m_Vertex1[3];
        float       m_Vertex2[3];
        float       m_Vertex3[3];
        uint16_t    m_Attr;
    } STLFacetT;
#pragma pack(pop)


class STLBObj {
    public:
       ~STLBObj();

        STLBObj(int threads = 2);

        STLBObj(const std::string &filename, int threads = 2);

        bool
        Dump(std::ostream & out = std::cout);

        bool
        Save(const std::string &filename);

        bool
        Translate(float x, float y, float z);

        bool
        FilterX(float x);

        bool
        FilterY(float y);

        bool
        FilterZ(float z);

        bool
        Add(const STLFacetT &facet);

        void
        Rotate(float x, float y, float z);

        void
        Scale(float x, float y, float z);

        void
        Centroid(float &x, float &y, float &z);

        void
        MinMax(
          float (&x)[2],
          float (&y)[2],
          float (&z)[2]
        );

        void
        Split();


    private:

        class Impl;
        std::unique_ptr<Impl> pimpl;
};
