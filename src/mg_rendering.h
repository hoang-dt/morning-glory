#pragma once

#include "mg_common.h"
#include "mg_algorithm.h"
#include "mg_array.h"
#include "mg_error.h"
#include "mg_io.h"
#include "mg_memory.h"
// TODO: rewrite to remove the dependence on picojson (which uses the STL)
#include "picojson.h"
#include "mg_scopeguard.h"
#include <string>

namespace mg {

struct transfer_func {
  array<f64> ValsRGB; // input to the RGB mapping, normalized to [0..1]
  array<f64> ValsAlpha; // input to the Alpha mapping, normalized to [0..1]
  array<v3d> RGB;
  array<f64> A;
};

inline void Dealloc(transfer_func* Tf) {
  Dealloc(&Tf->ValsRGB);
  Dealloc(&Tf->RGB);
  Dealloc(&Tf->A);
}

/* Load a JSON input file exported from ParaView, that contains a transfer function. */
inline error<>
ReadTransferFunc(cstr FileName, transfer_func* OutTf) {
  namespace pj = picojson;

  /* parse json file */
  buffer Buf;
  auto Result = ReadFile(FileName, &Buf);
  mg_CleanUp(0, DeallocBuf(&Buf));
  if (!Result)
    return Result;
  pj::value JsonVal;
  std::string Err = pj::parse(JsonVal, std::string((cstr)Buf.Data));
  if (!Err.empty())
    return mg_Error(err_code::ParseFailed);

  transfer_func TransFunc;
  const auto& Prgb = JsonVal.get<pj::array>()[0].get<pj::object>()["RGBPoints"];
  if (Prgb.evaluate_as_boolean()) {
    const auto& PointsRGB = Prgb.get<pj::array>();
    Resize(&TransFunc.ValsRGB, PointsRGB.size() / 4);
    Resize(&TransFunc.RGB    , PointsRGB.size() / 4);
    Resize(&TransFunc.A      , PointsRGB.size() / 4);
    /* extract RBG points*/
    for (int I = 0; I < Size(TransFunc.ValsRGB); ++I) {
      int J = I * 4;
      TransFunc.ValsRGB[I]   = PointsRGB[J    ].get<f64>();
      TransFunc.RGB    [I].R = PointsRGB[J + 1].get<f64>();
      TransFunc.RGB    [I].G = PointsRGB[J + 2].get<f64>();
      TransFunc.RGB    [I].B = PointsRGB[J + 3].get<f64>();
    }
  } else {
    return mg_Error(err_code::ParseFailed);
  }

  /* normalize the ValsRgb */
  min_max<f64*> MM = MinMaxElem(Begin(TransFunc.ValsRGB), End(TransFunc.ValsRGB));
  for (int I = 0; I < Size(TransFunc.ValsRGB); ++I) {
    auto& V = TransFunc.ValsRGB[I];
    V = (V - *(MM.Min)) / (*(MM.Max) - *(MM.Min));
  }

  const auto& P = JsonVal.get<pj::array>()[0].get<pj::object>()["Points"];
  if (P.evaluate_as_boolean()) {
    const auto& PointsA = P.get<pj::array>();
    Resize(&TransFunc.ValsAlpha, PointsA.size() / 4);
    Resize(&TransFunc.A        , PointsA.size() / 4);
    /* extract opacity points */
    for (int I = 0; I < Size(TransFunc.ValsAlpha); ++I) {
      int J = I * 4;
      TransFunc.ValsAlpha[I] = PointsA[J    ].get<f64>();
      TransFunc.A        [I] = PointsA[J + 1].get<f64>();
    }
  }

  *OutTf = TransFunc;
  return mg_Error(err_code::NoError);
}

// TODO: replace with a binary search
mg_Inline v3d
GetRGB(const transfer_func& Tf, f64 Val) {
  int J = 0;
  for (int I = 1; I < Size(Tf.ValsRGB); ++I) {
    if (Tf.ValsRGB[I] >= Val) {
      J = I;
      break;
    }
  }
  //printf("%f %f %f\n", Val, Tf.ValsRGB[J - 1], Tf.ValsRGB[J]);
  return Lerp(Tf.RGB[J - 1], Tf.RGB[J], (Val - Tf.ValsRGB[J - 1]) / (Tf.ValsRGB[J] - Tf.ValsRGB[J - 1]));
}

}
