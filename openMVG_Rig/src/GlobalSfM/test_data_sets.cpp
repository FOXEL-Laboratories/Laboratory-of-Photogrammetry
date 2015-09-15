/*
 * relative_translation_test
 *
 * Copyright (c) 2014-2015 FOXEL SA - http://foxel.ch
 * Please read <http://foxel.ch/license> for more information.
 *
 *
 * Author(s):
 *
 *      Pierre Moulon <p.moulon@foxel.ch>
 *      Stephane Flotron <s.flotron@foxel.ch>
 *
 * This file is part of the FOXEL project <http://foxel.ch>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Additional Terms:
 *
 *      You are required to preserve legal notices and author attributions in
 *      that material or in the Appropriate Legal Notices displayed by works
 *      containing it.
 *
 *      You are required to attribute the work as explained in the "Usage and
 *      Attribution" section of <http://foxel.ch/license>.
 */

#include <cmath>
#include <fstream>

#include "openMVG/numeric/numeric.h"
#include "openMVG/multiview/projection.hpp"
#include "openMVG/multiview/test_data_sets.hpp"

namespace openMVG {


nViewDatasetConfigurator::nViewDatasetConfigurator(int fx, int fy,
  int cx, int cy, double distance, double jitter_amount):
  _fx(fx), _fy(fy), _cx(cx), _cy(cy), _dist(distance),
  _jitter_amount(jitter_amount)
{}

NViewDataSet NRealisticCamerasRing(size_t nviews, size_t npoints,
                                   const nViewDatasetConfigurator config)
{
  //-- Setup a camera circle rig.
  NViewDataSet d;
  d._n = nviews;
  d._K.resize(nviews);
  d._R.resize(nviews);
  d._t.resize(nviews);
  d._C.resize(nviews);
  d._x.resize(nviews);
  d._x_ids.resize(nviews);

  d._X.resize(3, npoints);
  d._X.setRandom();
  d._X *= 0.6;

  Vecu all_point_ids(npoints);
  for (size_t j = 0; j < npoints; ++j)
    all_point_ids[j] = j;

  for (size_t i = 0; i < nviews; ++i) {
    Vec3 camera_center, t, jitter, lookdir;

    const double theta = i * 2 * M_PI / nviews;
    //-- Circle equation
    camera_center << sin(theta), 0.0, cos(theta); // Y axis UP
    camera_center *= config._dist;
    d._C[i] = camera_center;

    jitter.setRandom();
    jitter *= config._jitter_amount / camera_center.norm();
    lookdir = -camera_center + jitter;

    d._K[i] << config._fx,           0, config._cx,
                        0,  config._fy, config._cy,
                        0,           0,          1;
    d._R[i] = LookAt(lookdir);  // Y axis UP
    d._t[i] = -d._R[i] * camera_center; // [t]=[-RC] Cf HZ.
    d._x[i] = Project(d.P(i), d._X);
    d._x_ids[i] = all_point_ids;
  }
  return d;
}

Mat34 NViewDataSet::P(size_t i)const {
  assert(i < _n);
  Mat34 P;
  P_From_KRt(_K[i], _R[i], _t[i], &P);
  return P;
}

void NViewDataSet::ExportToPLY(
  const std::string & out_file_name)const {
  std::ofstream outfile;
  outfile.open(out_file_name.c_str(), std::ios_base::out);
  if (outfile.is_open()) {
    outfile << "ply"
     << std::endl << "format ascii 1.0"
     << std::endl << "comment NViewDataSet export"
     << std::endl << "comment It shows 3D point structure and cameras"
                  << "+ camera looking direction"
     << std::endl << "element vertex " << _X.cols() + _t.size()*2
     << std::endl << "property float x"
     << std::endl << "property float y"
     << std::endl << "property float z"
     << std::endl << "property uchar red"
     << std::endl << "property uchar green"
     << std::endl << "property uchar blue"
     << std::endl << "end_header" << std::endl;

    //-- Export 3D point cloud
    for(Mat3X::Index i = 0; i < _X.cols(); ++i) {
      // Exports the point position and point color
      outfile << _X.col(i).transpose()
        << " " << "255 255 255" << std::endl;
    }

    //-- Export 3D camera position t = -RC
    for(size_t i = 0; i < _t.size(); ++i) {
      // Exports the camera position and camera color
      outfile << (-_R[i].transpose()*_t[i]).transpose()
        << " " << "0 255 0" << std::endl;
    }
    for(size_t i = 0; i < _t.size(); ++i) {
      Vec3 test;
      test << 0, 0 , 0.4;
      // Exports the camera normal
      outfile << ((-_R[i].transpose()*_t[i])+
        (_R[i].transpose()*test)).transpose()
        << " " << "255 0 0" << std::endl;
    }
    outfile.close();
  }
}

NViewDataSet NRealisticCamerasCardioid(size_t nviews, size_t npoints,
                                        const nViewDatasetConfigurator config)
{
  //-- Setup a camera circle rig.
  NViewDataSet d;
  d._n = nviews;
  d._K.resize(nviews);
  d._R.resize(nviews);
  d._t.resize(nviews);
  d._C.resize(nviews);
  d._x.resize(nviews);
  d._x_ids.resize(nviews);

  d._X.resize(3, npoints);
  d._X.setRandom();
  d._X *= 0.6;

  Vecu all_point_ids(npoints);
  for (size_t j = 0; j < npoints; ++j)
    all_point_ids[j] = j;

  for (size_t i = 0; i < nviews; ++i) {
    Vec3 camera_center, t, jitter, lookdir;

    const double theta = i * 2 * M_PI / nviews;
    //-- Cardioid
    camera_center <<
      2*sin(theta)-(sin(2*theta)),
      0.0,
      2*cos(theta)-(cos(2*theta)); // Y axis UP
    camera_center *= config._dist;
    d._C[i] = camera_center;

    jitter.setRandom();
    jitter *= config._jitter_amount / camera_center.norm();
    lookdir = -camera_center + jitter;

    d._K[i] << config._fx,           0, config._cx,
      0,  config._fy, config._cy,
      0,           0,          1;
    d._R[i] = LookAt(lookdir);  // Y axis UP
    d._t[i] = -d._R[i] * camera_center; // [t]=[-RC] Cf HZ.
    d._x[i] = Project(d.P(i), d._X);
    d._x_ids[i] = all_point_ids;
  }
  return d;
}

}  // namespace openMVG
