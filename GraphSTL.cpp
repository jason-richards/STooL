#include "GraphSTL.hpp"

#include <map>
#include <list>
#include <tuple>
#include <vector>


namespace GraphSTL {


class Triangle {
public:
  using VertexID        = std::tuple<float, float, float>;
  using VertexList      = std::list<VertexID>;
  using TriangleID      = std::tuple<VertexID, VertexID, VertexID>;
  using TrianglePtr     = std::shared_ptr<Triangle>;
  using TrianglePtrList = std::list<TrianglePtr>;

  static TrianglePtr
  Create(
    float normal[3],
    float vertex1[3],
    float vertex2[3],
    float vertex3[3],
    uint16_t attributes
  ) {
    return std::make_shared<Triangle>(
      normal, vertex1, vertex2, vertex3, attributes
    );
  }

  Triangle (
    float normal[3],
    float vertex1[3],
    float vertex2[3],
    float vertex3[3],
    uint16_t attributes
  ) :
    m_Normal (std::make_tuple(normal[0],  normal[1],  normal[2])),
    m_Vertex1(std::make_tuple(vertex1[0], vertex1[1], vertex1[2])),
    m_Vertex2(std::make_tuple(vertex2[0], vertex2[1], vertex2[2])),
    m_Vertex3(std::make_tuple(vertex3[0], vertex3[1], vertex3[2])),
    m_ID(std::make_tuple(m_Vertex1, m_Vertex2, m_Vertex3)),
    m_Attributes(attributes) {}

  VertexID m_Normal;
  VertexID m_Vertex1;
  VertexID m_Vertex2;
  VertexID m_Vertex3;
  TriangleID m_ID;
  uint16_t m_Attributes;
};


class Context {
public:
  using ContextPtr = std::shared_ptr<Context>;

  static ContextPtr
  Create() {
    return std::make_shared<Context>();
  }


  Context() {};


  Triangle::TrianglePtr
  AddTriangle(
    float normal[3],
    float vertex1[3],
    float vertex2[3],
    float vertex3[3],
    uint16_t attributes
  ) {
    auto triangle = Triangle::Create(normal, vertex1, vertex2, vertex3, attributes);
    m_VertexTriangles[triangle->m_Vertex1].push_back(triangle);
    m_VertexTriangles[triangle->m_Vertex2].push_back(triangle);
    m_VertexTriangles[triangle->m_Vertex3].push_back(triangle);
    m_Triangles[triangle->m_ID] = triangle;
    m_TriangleVertices[triangle->m_ID].push_back(triangle->m_Vertex1);
    m_TriangleVertices[triangle->m_ID].push_back(triangle->m_Vertex2);
    m_TriangleVertices[triangle->m_ID].push_back(triangle->m_Vertex3);
    return triangle;
  }


  void
  AllTriangles (
    Triangle::TrianglePtrList& triangles
  ) {
      transform(m_Triangles.begin(), m_Triangles.end(), back_inserter(triangles),
      [](const std::map<Triangle::TriangleID, Triangle::TrianglePtr>::value_type& val) {
        return val.second;
      }
    );
  }


  void
  AdjacentTriangles (
    const Triangle::TrianglePtr& reference,
    std::map<Triangle::TriangleID, Triangle::TrianglePtr>& score_board,
    Triangle::TrianglePtrList& triangles
  ) {
    for (const auto& vertice : m_TriangleVertices[reference->m_ID]) {
      std::copy_if(m_VertexTriangles[vertice].begin(),
      m_VertexTriangles[vertice].end(),
        std::back_inserter(triangles),
        [&](const auto& a) {
          return !score_board.contains(a->m_ID) && a->m_ID != reference->m_ID;
        }
      );
    }
  }


  void
  TraverseAdjacent(
    const Triangle::TrianglePtr& triangle,
    Triangle::TrianglePtrList& connected,
    std::map<Triangle::TriangleID, Triangle::TrianglePtr>& score_board
  ) {
    score_board[triangle->m_ID] = triangle;

    Triangle::TrianglePtrList adjacency_list;
    AdjacentTriangles(triangle, score_board, adjacency_list);

    for (const auto& adjacent : adjacency_list) {
      if (!score_board.contains(adjacent->m_ID)) {
        connected.push_back(adjacent);
        score_board[adjacent->m_ID] = adjacent;
        TraverseAdjacent(adjacent, connected, score_board);
      }
    }
  }


  void
  ManifoldObjects(
    std::list<Triangle::TrianglePtrList>& objects
  ) {
    std::map<Triangle::TriangleID, Triangle::TrianglePtr> score_board;

    Triangle::TrianglePtrList triangles;
    AllTriangles(triangles);

    for (const auto& triangle : triangles) {
      if (!score_board.contains(triangle->m_ID)) {
        Triangle::TrianglePtrList adjacency_list {triangle};
        TraverseAdjacent(triangle, adjacency_list, score_board);
        objects.push_back(adjacency_list);
      }
    }
  }

  std::map<Triangle::VertexID, Triangle::TrianglePtrList> m_VertexTriangles;
  std::map<Triangle::TriangleID, Triangle::VertexList>    m_TriangleVertices;
  std::map<Triangle::TriangleID, Triangle::TrianglePtr>   m_Triangles;
};


ContextPtr
CreateContext() {
  return Context::Create();
}


void
Extract (
  const TrianglePtr& triangle,
  float (&normal)[3],
  float (&vertex1)[3],
  float (&vertex2)[3],
  float (&vertex3)[3],
  uint16_t& attributes
) {
  normal[0] = std::get<0>(triangle->m_Normal);
  normal[1] = std::get<1>(triangle->m_Normal);
  normal[2] = std::get<2>(triangle->m_Normal);

  vertex1[0] = std::get<0>(triangle->m_Vertex1);
  vertex1[1] = std::get<1>(triangle->m_Vertex1);
  vertex1[2] = std::get<2>(triangle->m_Vertex1);

  vertex2[0] = std::get<0>(triangle->m_Vertex2);
  vertex2[1] = std::get<1>(triangle->m_Vertex2);
  vertex2[2] = std::get<2>(triangle->m_Vertex2);

  vertex3[0] = std::get<0>(triangle->m_Vertex3);
  vertex3[1] = std::get<1>(triangle->m_Vertex3);
  vertex3[2] = std::get<2>(triangle->m_Vertex3);

  attributes = triangle->m_Attributes;
}


Triangle::TrianglePtr
AddTriangle(
  ContextPtr ctx,
  float normal[3],
  float vertex1[3],
  float vertex2[3],
  float vertex3[3],
  uint16_t attributes
) {
  return ctx->AddTriangle(normal, vertex1, vertex2, vertex3, attributes);
}


void
ManifoldObjects(
  ContextPtr ctx,
  std::list<Triangle::TrianglePtrList>& objects
) {
  ctx->ManifoldObjects(objects);
}

} /* namespace GraphSTL */
