#pragma once

#include <memory>
#include <list>

namespace GraphSTL {


class Context;
using ContextPtr = std::shared_ptr<Context>;


class Triangle;
using TrianglePtr = std::shared_ptr<Triangle>;
using TrianglePtrList = std::list<TrianglePtr>;


ContextPtr
CreateContext();

void
Extract (
  const TrianglePtr& triangle,
  float (&normal)[3],
  float (&vertex1)[3],
  float (&vertex2)[3],
  float (&vertex3)[3],
  uint16_t& attributes
);


TrianglePtr
AddTriangle (
  ContextPtr ctx,
  float normal[3],
  float vertex1[3],
  float vertex2[3],
  float vertex3[3],
  uint16_t attributes = 0
);

void
ManifoldObjects(
  ContextPtr ctx,
  std::list<TrianglePtrList>& objects
);


} /* namespace GraphSTL */
