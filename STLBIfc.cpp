#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cmath>
#include <functional>

#include "STLBIfc.hpp"

#define STLB_BLOCK_SIZE 4096

#pragma pack(push, 1)
    typedef struct
    STLHeader {
        char        m_Header[80];
        uint32_t    m_Facets;
    } STLHeaderT;
#pragma pack(pop)


// Apply rotation about X axis
//    [ 1     0      0   ]  [x]
//    [ 0     cos@  -sin@]  [y]
//    [ 0     sin@   cos@]  [z]
static inline void
RotateX(
  const float &angle,
  float (&xyz)[3]
) {
  float yN = ( std::cos(angle) * xyz[1]) +
             (-std::sin(angle) * xyz[2]);
  float zN = ( std::sin(angle) * xyz[1]) +
             ( std::cos(angle) * xyz[2]);
  xyz[1] = yN;
  xyz[2] = zN;
}


// Apply rotation about Y axis
//    [ cos@  0     sin@]
//    [ 0     1     0   ]
//    [-sin@  0     cos@]
static inline void
RotateY(
  const float &angle,
  float (&xyz)[3]
) {
  float xN = ( std::cos(angle) * xyz[0]) +
             ( std::sin(angle) * xyz[2]);
  float zN = (-std::sin(angle) * xyz[0]) +
             ( std::cos(angle) * xyz[2]);
  xyz[0] = xN;
  xyz[2] = zN;
}


// Apply rotation about Z axis
//    [ cos@ -sin@  0   ]
//    [ sin@  cos@  0   ]
//    [ 0     0     1   ]
static inline void
RotateZ(
  const float &angle,
  float (&xyz)[3]
) {
  float xN = ( std::cos(angle) * xyz[0]) +
             (-std::sin(angle) * xyz[1]);
  float yN = ( std::sin(angle) * xyz[0]) +
             ( std::cos(angle) * xyz[1]);
  xyz[0] = xN;
  xyz[1] = yN;
}


class STLBObj::Impl {
    public:
        Impl(int threads) : m_Threads(threads) {
            buffer.reserve(STLB_BLOCK_SIZE);
            const char * IDENT = "STLB Reader/Writer";
            buffer.resize(sizeof(STLHeaderT));
            std::strcpy(const_cast<char*>(&buffer[0]), IDENT);
        }


        Impl(
            const std::string& filename,
            int threads
        ) : m_Threads(threads) {
            std::ifstream input{filename};

            input.seekg(0, input.end);
            size_t length = input.tellg();
            input.seekg(0, input.beg);

            if (length > 0 && length > sizeof(STLHeaderT)) {
                buffer.resize(length);
                input.read(&buffer[0], length);
            }

            input.close();

            auto f = GetNFacets();

            if (buffer.size() != (f * sizeof(STLFacetT) + sizeof(STLHeaderT))) {
                std::cerr << "Invalid or Corrupt STLB. " << buffer.size() << " != "
                          << (f * sizeof(STLFacetT) + sizeof(STLHeaderT)) << std::endl;
                return;
            }
        }


        bool
        Dump(
            std::ostream & out
        ) {
            STLHeaderT * h = GetHeader();

            out << "\tHeader:  " << h->m_Header << std::endl;
            out << "\tFacets:  " << h->m_Facets << std::endl;

            STLFacetT * facets = GetFacets();
            auto nFacets = GetNFacets();
            for (int i = 0; i < nFacets; i++) {
                out << "Facet: " << i << std::endl;

                if (facets[i].m_Attr != 0) {
                    out << "Invalid or Corrupt STLB." << std::endl;
                    return false;
                }

                out << "\tNormal: "
                    << facets[i].m_Normal[0] << ", "
                    << facets[i].m_Normal[1] << ", "
                    << facets[i].m_Normal[2] << std::endl;

                out << "\tVertex1: "
                    << facets[i].m_Vertex1[0] << ", "
                    << facets[i].m_Vertex1[1] << ", "
                    << facets[i].m_Vertex1[2] << std::endl;

                out << "\tVertex2: "
                    << facets[i].m_Vertex2[0] << ", "
                    << facets[i].m_Vertex2[1] << ", "
                    << facets[i].m_Vertex2[2] << std::endl;

                out << "\tVertex3: "
                    << facets[i].m_Vertex3[0] << ", "
                    << facets[i].m_Vertex3[1] << ", "
                    << facets[i].m_Vertex3[2] << std::endl;
            }

            return true;
        }


        bool
        Save(
            const std::string &filename
        ) {
            std::ofstream output{filename, std::ios::binary | std::ios::out};
            output.write(reinterpret_cast<char*>(&buffer[0]), buffer.size());
            output.close();
            return true;
        }


        void
        MinMax (
          float (&x)[2],
          float (&y)[2],
          float (&z)[2]
        ) {
          STLFacetT * facets = GetFacets();
          int nFacets = GetNFacets();
          for (int i = 0; i < nFacets; i++) {
            if (i == 0) {
              x[0] = x[1] = facets[i].m_Vertex1[0];
              y[0] = y[1] = facets[i].m_Vertex1[1];
              z[0] = z[1] = facets[i].m_Vertex1[2];
            }

            // X-min
            if (facets[i].m_Vertex1[0] < x[0]) x[0] = facets[i].m_Vertex1[0];
            if (facets[i].m_Vertex2[0] < x[0]) x[0] = facets[i].m_Vertex2[0];
            if (facets[i].m_Vertex3[0] < x[0]) x[0] = facets[i].m_Vertex3[0];

            // X-max
            if (facets[i].m_Vertex1[0] > x[1]) x[1] = facets[i].m_Vertex1[0];
            if (facets[i].m_Vertex2[0] > x[1]) x[1] = facets[i].m_Vertex2[0];
            if (facets[i].m_Vertex3[0] > x[1]) x[1] = facets[i].m_Vertex3[0];

            // Y-min
            if (facets[i].m_Vertex1[1] < y[0]) y[0] = facets[i].m_Vertex1[1];
            if (facets[i].m_Vertex2[1] < y[0]) y[0] = facets[i].m_Vertex2[1];
            if (facets[i].m_Vertex3[1] < y[0]) y[0] = facets[i].m_Vertex3[1];

            // Y-max
            if (facets[i].m_Vertex1[1] > y[1]) y[1] = facets[i].m_Vertex1[1];
            if (facets[i].m_Vertex2[1] > y[1]) y[1] = facets[i].m_Vertex2[1];
            if (facets[i].m_Vertex3[1] > y[1]) y[1] = facets[i].m_Vertex3[1];

            // Z-min
            if (facets[i].m_Vertex1[2] < z[0]) z[0] = facets[i].m_Vertex1[2];
            if (facets[i].m_Vertex2[2] < z[0]) z[0] = facets[i].m_Vertex2[2];
            if (facets[i].m_Vertex3[2] < z[0]) z[0] = facets[i].m_Vertex3[2];

            // Z-max
            if (facets[i].m_Vertex1[2] > z[1]) z[1] = facets[i].m_Vertex1[2];
            if (facets[i].m_Vertex2[2] > z[1]) z[1] = facets[i].m_Vertex2[2];
            if (facets[i].m_Vertex3[2] > z[1]) z[1] = facets[i].m_Vertex3[2];
          }
        }


        void
        Centroid (
          float &x,
          float &y,
          float &z
        ) {
          float xMinMax[2] = { 0, 0 };
          float yMinMax[2] = { 0, 0 };
          float zMinMax[2] = { 0, 0 };

          MinMax(xMinMax, yMinMax, zMinMax);

          x = ((xMinMax[0] + xMinMax[1]) / 2);
          y = ((yMinMax[0] + yMinMax[1]) / 2);
          z = ((zMinMax[0] + zMinMax[1]) / 2);
        }


        bool
        Translate(
            float x,
            float y,
            float z
        ) {
            STLFacetT * facets = GetFacets();
            int nFacets = GetNFacets();
            for (int i = 0; i < nFacets; i++) {
                facets[i].m_Vertex1[0] += x;
                facets[i].m_Vertex1[1] += y;
                facets[i].m_Vertex1[2] += z;

                facets[i].m_Vertex2[0] += x;
                facets[i].m_Vertex2[1] += y;
                facets[i].m_Vertex2[2] += z;

                facets[i].m_Vertex3[0] += x;
                facets[i].m_Vertex3[1] += y;
                facets[i].m_Vertex3[2] += z;
            }

            return true;
        }


        void
        Scale(
          float x,
          float y,
          float z
        ) {
          STLFacetT * facets = GetFacets();
          int nFacets = GetNFacets();

          float xyzCentroid[3];

          Centroid(xyzCentroid[0], xyzCentroid[1], xyzCentroid[2]);

          Translate(-xyzCentroid[0], -xyzCentroid[1], -xyzCentroid[2]);

          for (int i = 0; i < nFacets; i++) {
              facets[i].m_Normal[0]  *= x;
              facets[i].m_Normal[1]  *= y;
              facets[i].m_Normal[2]  *= z;

              facets[i].m_Vertex1[0] *= x;
              facets[i].m_Vertex1[1] *= y;
              facets[i].m_Vertex1[2] *= z;

              facets[i].m_Vertex2[0] *= x;
              facets[i].m_Vertex2[1] *= y;
              facets[i].m_Vertex2[2] *= z;

              facets[i].m_Vertex3[0] *= x;
              facets[i].m_Vertex3[1] *= y;
              facets[i].m_Vertex3[2] *= z;
          }

          Translate(xyzCentroid[0], xyzCentroid[1], xyzCentroid[2]);
        }


        bool
        Filter(
            std::function<bool(const STLFacetT &, float v)> cmp,
            float value
        ) {
            auto header = GetHeader();
            auto src = GetFacets();
            auto dst = GetFacets();
            int nFacets = GetNFacets();
            int index = 0;

            for (int i = 0; i < nFacets; i++) {
                if (cmp(src[i], value)) {
                    std::memcpy(reinterpret_cast<char *>(&dst[index++]),
                                reinterpret_cast<char *>(&src[i]),
                                sizeof(STLFacetT));
                }
            }

            header->m_Facets = index;

            return true;
        }


        bool
        Add(
            const STLFacetT &facet
        ) {
            auto header = GetHeader();
            auto facets = GetNFacets();

            auto offset = sizeof(STLHeaderT) + (facets * sizeof(STLFacetT));

            if (buffer.size() < (offset + sizeof(STLFacet))) {
                buffer.resize(buffer.size()*2);
            }

            std::memcpy(reinterpret_cast<char *>(&buffer[offset]),
                        reinterpret_cast<const char *>(&facet),
                        sizeof(STLFacetT));

            header->m_Facets++;

            return true;
        }


        void
        Rotate(
            float x,
            float y,
            float z
        ) {
            STLFacetT * facets = GetFacets();
            int nFacets = GetNFacets();
            for (int i = 0; i < nFacets; i++) {
                if (x != 0.0) {
                  RotateX(x, facets[i].m_Normal);
                  RotateX(x, facets[i].m_Vertex1);
                  RotateX(x, facets[i].m_Vertex2);
                  RotateX(x, facets[i].m_Vertex3);
                }

                if (y != 0.0) {
                  RotateY(y, facets[i].m_Normal);
                  RotateY(y, facets[i].m_Vertex1);
                  RotateY(y, facets[i].m_Vertex2);
                  RotateY(y, facets[i].m_Vertex3);
                }

                if (z != 0.0) {
                  RotateZ(z, facets[i].m_Normal);
                  RotateZ(z, facets[i].m_Vertex1);
                  RotateZ(z, facets[i].m_Vertex2);
                  RotateZ(z, facets[i].m_Vertex3);
                }
            }
        }


        void
        Split() {
          GraphSTL::ContextPtr context = GraphSTL::CreateContext();

          STLFacetT * facets = GetFacets();
          int nFacets = GetNFacets();

          for (int i = 0; i < nFacets; i++) {
            GraphSTL::AddTriangle(
              context,
              facets[i].m_Normal,
              facets[i].m_Vertex1,
              facets[i].m_Vertex2,
              facets[i].m_Vertex3
            );
          }

          std::list<GraphSTL::TrianglePtrList> manifold_objects;
          GraphSTL::ManifoldObjects(context, manifold_objects);

          STLHeaderT objHeader;
          STLFacetT  objFacet;

          std::memset(reinterpret_cast<char*>(&objHeader), 0, sizeof(STLHeaderT));
          const char * IDENT = "STLB Reader/Writer";
          std::strcpy(objHeader.m_Header, IDENT);

          size_t n = 0;
          for (const auto& object : manifold_objects) {
            std::stringstream outname;
            outname << "manifold_object_" << n++ << ".stl";

            std::ofstream output{outname.str().c_str(),
              std::ios::binary | std::ios::out};

            objHeader.m_Facets = object.size();

            output.write(reinterpret_cast<char*>(&objHeader), sizeof(STLHeaderT));

            for (const auto& triangle : object) {
              std::memset(reinterpret_cast<char*>(&objFacet), 0, sizeof(STLFacetT));

              GraphSTL::Extract(
                triangle,
                objFacet.m_Normal,
                objFacet.m_Vertex1,
                objFacet.m_Vertex2,
                objFacet.m_Vertex3,
                objFacet.m_Attr
              );

              output.write(reinterpret_cast<char*>(&objFacet), sizeof(STLFacetT));
            }

            output.close();
          }
        }

    private:

        STLHeaderT *
        GetHeader() {
            return reinterpret_cast<STLHeaderT *>(&buffer[0]);
        }


        int
        GetNFacets() {
            auto header = GetHeader();
            return header->m_Facets;
        }


        STLFacetT *
        GetFacets() {
            return reinterpret_cast<STLFacetT *>(&buffer[sizeof(STLHeaderT)]);
        }

        int m_Threads;
        std::vector<char> buffer;
};


STLBObj::~STLBObj() {}


STLBObj::STLBObj(int threads) : pimpl(new STLBObj::Impl(threads)) {}


STLBObj::STLBObj(
    const std::string &filename,
    int threads
) : pimpl(new STLBObj::Impl(filename, threads)) {}


bool
STLBObj::Dump(
    std::ostream & out
) {
    return pimpl->Dump(out);
}


bool
STLBObj::Translate(
    float x,
    float y,
    float z
) {
    return pimpl->Translate(x, y, z);
}


bool
STLBObj::Save(
    const std::string &filename
) {
    return pimpl->Save(filename);
}


bool
STLBObj::FilterX(float x) {
    return pimpl->Filter(
        [](const STLFacetT &facet, float value) {
            return facet.m_Vertex1[0] == value && facet.m_Vertex2[0] == value && facet.m_Vertex3[0] == value;
        }, x);
}


bool
STLBObj::FilterY(float y) {
    return pimpl->Filter(
        [](const STLFacetT &facet, float value) {
            return facet.m_Vertex1[1] == value && facet.m_Vertex2[1] == value && facet.m_Vertex3[1] == value;
        }, y);
}


bool
STLBObj::FilterZ(float z) {
    return pimpl->Filter(
        [](const STLFacetT &facet, float value) {
            return facet.m_Vertex1[2] == value && facet.m_Vertex2[2] == value && facet.m_Vertex3[2] == value;
        }, z);
}


bool
STLBObj::Add(const STLFacetT &facet) {
    return pimpl->Add(facet);
}


void
STLBObj::Centroid(
  float &x,
  float &y,
  float &z
) {
  pimpl->Centroid(x, y, z);
}


void
STLBObj::MinMax(
  float (&x)[2],
  float (&y)[2],
  float (&z)[2]
) {
  pimpl->MinMax(x, y, z);
}

void
STLBObj::Rotate (
  float x,
  float y,
  float z
) {
  pimpl->Rotate(x, y, z);
}


void
STLBObj::Scale(
  float x,
  float y,
  float z
) {
  pimpl->Scale(x, y, z);
}


void
STLBObj::Split() {
  pimpl->Split();
}
