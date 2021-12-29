#include <boost/program_options.hpp>
#include <iostream>
#include <unistd.h>
#include "STLBIfc.hpp"

namespace bpo = boost::program_options;
int
main (
    int           argc,
    const char  **argv
) {
  bpo::options_description desc;

  std::string input;
  std::string output;

  desc.add_options()
   ("help,h",       "Help Screen.")
   ("centroid,c",   "Calculate and display centroid.")
   ("minmax,m",     "Calculate and display 3-plane min/max.")
   ("dump,d",       "Dump STL contents.")
   ("input,i",      bpo::value(&input)->default_value("input.stl"),
      "Specify input STL file.  EG: --input input.stl"  )
   ("output,o",     bpo::value(&output),
     "Specify output STL file. EG: --output output.stl")
   ("rotate,r",     bpo::value<std::string>(),
     "Specify 3-plane angle (DEGREES) of rotation.  EG: --rotate [float,float,float|x,y,z]")
   ("scale,sc",     bpo::value<std::string>(),
     "Specify 3-plane scaling factor.  EG: --scale [float,float,float|x,y,z]")
   ("split,sp",
     "Split manifold objects into separate STL files.")
   ("threads,th",   bpo::value<int>()->default_value(2),
     "Specify the number of threads to use. DEFAULT : 2")
   ("translate,t",  bpo::value<std::string>(),
     "Specify Translation Vector. EG: --translate [float,float,float|x,y,z]");

  bpo::variables_map vm;
  bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
  bpo::notify(vm);

  if (access(input.c_str(), F_OK) == -1 ) {
    std::cerr << "ERROR: Invalid filename: " << input << std::endl;
    return -1;
  }

  STLBObj source_stl(input, vm["threads"].as<int>());

  if (vm.count("centroid")) {
    float x = 0, y = 0, z = 0;
    source_stl.Centroid(x, y, z);
    std::cout
      << "X,Y,Z Centroid: "
      << x << ","
      << y << ","
      << z << std::endl;
  }

  if (vm.count("minmax")) {
    float x[2];
    float y[2];
    float z[2];
    source_stl.MinMax(x, y, z);
    std::cout
      << "X min: " << x[0] << ", X max: " << x[1] << std::endl
      << "Y min: " << y[0] << ", X max: " << y[1] << std::endl
      << "Z min: " << z[0] << ", X max: " << z[1] << std::endl;

  }

  if (vm.count("dump")) {
    source_stl.Dump(std::cout);
  }

  if (vm.count("rotate")) {
    std::istringstream ss(vm["rotate"].as<std::string>());
    std::string token;
    std::vector<std::string> axis_angle;
    while(std::getline(ss, token, ',')) {
      axis_angle.push_back(token);
    }

    try {
      if (axis_angle.size() != 3) {
        throw std::runtime_error("Expected 3 arguments.");
      }

      float xAngleDegrees = std::stof(axis_angle[0]) * (M_PI/180);
      float yAngleDegrees = std::stof(axis_angle[1]) * (M_PI/180);
      float zAngleDegrees = std::stof(axis_angle[2]) * (M_PI/180);

      source_stl.Rotate(xAngleDegrees, yAngleDegrees, zAngleDegrees);
    } catch (const std::exception& ex) {
      std::cerr << "Rotate Argument ERROR: \
Expected 3 comma separated values (DEGREES):  EG:  [90,0.0,0.0 | x,y,z]" << std::endl;
      return -1;
    }
  }

  if (vm.count("translate")) {
    std::istringstream ss(vm["translate"].as<std::string>());
    std::string token;
    std::vector<std::string> coordinate;
    while(std::getline(ss, token, ',')) {
      coordinate.push_back(token);
    }

    if (coordinate.size() != 3) {
      std::cerr << "Translate Argument ERROR: \
Expected 3 comma separates floats:  EG: 1.1, 2, 10" << std::endl;
      return -1;
    }

    source_stl.Translate(
      std::stof(coordinate[0]),
      std::stof(coordinate[1]),
      std::stof(coordinate[2])
    );
  }

  if (vm.count("scale")) {
    std::istringstream ss(vm["scale"].as<std::string>());
    std::string token;
    std::vector<std::string> scale_factor;
    while(std::getline(ss, token, ',')) {
      scale_factor.push_back(token);
    }

    if (scale_factor.size() != 3) {
      std::cerr << "Scale Argument ERROR: \
Expected 3 comma separates floats:  EG: 10,1,1" << std::endl;
      return -1;
    }

    source_stl.Scale(
      std::stof(scale_factor[0]),
      std::stof(scale_factor[1]),
      std::stof(scale_factor[2])
    );

  }

  if (vm.count("split")) {
    source_stl.Split();
  }

  if (vm.count("output")) {
    source_stl.Save(output);
  }

  if (vm.count("help")) {
    std::cout << desc;
  }

  //o.Split();
  return 0;
}
