// Copyright (C) 2014 - 2016  Jack O. Wasey
//
// This file is part of icd.
//
// icd is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// icd is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with icd. If not, see <http://www.gnu.org/licenses/>.

// [[Rcpp::interfaces(r, cpp)]]
#include "convert.h"
// util.h for strimCpp
#include "util.h"
#include "is.h"
// for add leading zeroes mjr (TODO: remove this dependency)
#include "manip.h"

//' Convert mjr and mnr vectors to single code
//'
//' In debug mode, will check that mjr and mnr are same length.
//' @template mjr
//' @template mnr
//' @template isShort
//' @examples
//' \dontrun{
//' n <- 5
//' mjrs <- as.character(sample(1:999, n, replace = TRUE))
//' mnrs <- as.character(sample(0:99, n, replace = TRUE))
//' microbenchmark::microbenchmark(
//'   icd9MajMinToCode(mjrs, mnrs, TRUE),
//'   icd9MajMinToCodeStd(mjrs, mnrs, TRUE),
//'   times = 3
//' )
//' }
//' # std method vastly quicker, e.g. x100 when n=5000
//' @keywords internal manip
// [[Rcpp::export]]
Rcpp::CharacterVector icd9MajMinToCode(const Rcpp::CharacterVector mjr,
                                       const Rcpp::CharacterVector mnr, bool isShort) {
#ifdef ICD_DEBUG_TRACE
  Rcpp::Rcout << "icd9MajMinToCode: mjr.size() = " << mjr.size()
              << " and mnr.size() = " << mnr.size() << "\n";
#endif
#ifdef ICD_DEBUG
  if (mjr.size() != mnr.size())
    Rcpp::stop("mjr and mnr lengths differ");
#endif

  std::vector<std::string> out(mjr.size());
  std::vector<char> out_is_na(mjr.size()); // boolean in char
  Rcpp::CharacterVector::const_iterator j = mjr.begin();
  Rcpp::CharacterVector::const_iterator n = mnr.begin();

  for (; j != mjr.end() && n != mnr.end(); ++j, ++n) {
    Rcpp::String mjrelem = *j;
    if (Rcpp::CharacterVector::is_na(*j)) {
      out_is_na[std::distance(mjr.begin(), j)] = 1;
      continue;
    }
    const char* smj_c = mjrelem.get_cstring();
    std::string smj = std::string(smj_c);
    switch (strlen(smj_c)) {
    case 0:
      out_is_na[std::distance(mjr.begin(), j)] = 1;
      continue;
    case 1:
      if (!icd9IsASingleVE(smj_c)) {
        smj.insert(0, "00");
      }
      break;
    case 2:
      if (!icd9IsASingleVE(smj_c)) {
        smj.insert(0, "0");
      } else {
        smj.insert(1, "0");
      }
      // default: // mjr is 3 (or more) chars already
    }
    Rcpp::String mnrelem = *n;
    if (Rcpp::CharacterVector::is_na(*n)) {
      mnrelem = "";
    }
    if (!isShort && mnrelem != "") {
      smj.append(".");
    }
    smj.append(mnrelem);
    out[std::distance(mjr.begin(), j)] = smj;
  }
  Rcpp::CharacterVector r_out = Rcpp::wrap(out);
#ifdef ICD_DEBUG_TRACE
  Rcpp::Rcout << "NA loop size: " << out_is_na.size() << "\n";
#endif
  for (std::vector<char>::iterator i = out_is_na.begin(); i != out_is_na.end(); ++i) {
#ifdef ICD_DEBUG_TRACE
    Rcpp::Rcout << "NA loop: " << std::distance(out_is_na.begin(), i) << "\n";
#endif
    if (*i == 0)
      continue;
    r_out[std::distance(out_is_na.begin(), i)] = NA_STRING;
  }
  return r_out;
}

// [[Rcpp::export]]
Rcpp::CharacterVector icd9MajMinToShort(const Rcpp::CharacterVector mjr,
                                        const Rcpp::CharacterVector mnr) {
#ifdef ICD_DEBUG_TRACE
  Rcpp::Rcout << "icd9MajMinToShort: mjr.size() = " << mjr.size()
              << " and mnr.size() = " << mnr.size() << "\n";
#endif
#ifdef ICD_DEBUG
  if ((mjr.size() != 1 && mjr.size() != mnr.size())
        || (mjr.size() == 1 && mnr.size() == 0)) {
    Rcpp::stop(
      "icd9MajMinToShort, length of mjrs and mnrs must be equal, unless mjrs length is one.");
  }
#endif
  if (mjr.size() == 1) {
#ifdef ICD_DEBUG_TRACE
    Rcpp::Rcout << "icd9MajMinToShort: mjr.size() = 1\n";
#endif
    Rcpp::CharacterVector newmjr(mnr.size(), mjr[0]);
    return icd9MajMinToCode(newmjr, mnr, true);
  }
  return icd9MajMinToCode(mjr, mnr, true);
}

// [[Rcpp::export]]
Rcpp::CharacterVector icd9MajMinToDecimal(const Rcpp::CharacterVector mjr,
                                          const Rcpp::CharacterVector mnr) {
  return icd9MajMinToCode(mjr, mnr, false);
}

//' @rdname convert
//' @keywords internal manip
// [[Rcpp::export]]
Rcpp::CharacterVector icd9PartsToShort(const Rcpp::List parts) {
  return icd9MajMinToCode(parts["mjr"], parts["mnr"], true);
}

//' @rdname convert
//' @keywords internal manip
// [[Rcpp::export]]
Rcpp::CharacterVector icd9PartsToDecimal(const Rcpp::List parts) {
  return icd9MajMinToCode(parts["mjr"], parts["mnr"], false);
}

// [[Rcpp::export]]
Rcpp::List icd9MajMinToParts(const Rcpp::CharacterVector mjr,
                             const Rcpp::CharacterVector mnr) {
  Rcpp::List returned_frame = Rcpp::List::create(Rcpp::_["mjr"] = mjr,
                                                 Rcpp::_["mnr"] = mnr);

  Rcpp::StringVector sample_row = returned_frame(0);
  Rcpp::IntegerVector row_names = seq_along(sample_row);
  returned_frame.attr("row.names") = row_names;
  // doesn't actually need a data frame, although it is barely distinguishable
  // from a list, and not costly to construct in this manner.
  returned_frame.attr("class") = "data.frame";

  return returned_frame;
}

//' @rdname convert
//' @keywords internal manip
// [[Rcpp::export]]
Rcpp::List icd9ShortToPartsCpp(Rcpp::CharacterVector icd9Short, Rcpp::String mnrEmpty) {

  Rcpp::CharacterVector mjr(icd9Short.size());
  Rcpp::CharacterVector mnr(icd9Short.size());

  for (int i = 0; i < icd9Short.size(); ++i) {
    Rcpp::String thisShort = icd9Short[i];
    if (thisShort == NA_STRING) { // .is_na() is private?
      mnr[i] = NA_STRING; // I think set_na() might be an alternative.
      continue;
    }

    std::string s(thisShort.get_cstring());
    s = strimCpp(s); // in place or rewrite?
    std::string::size_type sz = s.size();

    if (icd9IsASingleE(s.c_str())) { // E code
      switch (sz) {
      case 2:
      case 3:
      case 4:
        mjr[i] = s.substr(0, sz);
        mnr[i] = mnrEmpty;
        break;
      case 5:
        mjr[i] = s.substr(0, 4);
        mnr[i] = s.substr(4, 1);
        break;
      default:
        mjr[i] = NA_STRING;
      mnr[i] = NA_STRING;
      continue;
      }
    } else { // not an E code
      switch (sz) {
      case 1:
      case 2:
      case 3:
        mjr[i] = s.substr(0, sz);
        mnr[i] = mnrEmpty;
        continue;
      case 4:
      case 5:
        mjr[i] = s.substr(0, 3);
        mnr[i] = s.substr(3, sz - 3);
        continue;
      default:

        mjr[i] = NA_STRING;
      mnr[i] = NA_STRING;
      continue;
      }
    }

  } // for

  return icd9MajMinToParts(icd9AddLeadingZeroesMajor(mjr), mnr);
}

//' @describeIn icd_decimal_to_parts Convert short ICD-10 code to parts
//' @export
//' @keywords internal manip
// [[Rcpp::export(icd_short_to_parts.icd10)]]
Rcpp::List icd10ShortToPartsCpp(const Rcpp::CharacterVector x, const Rcpp::String mnr_empty = "") {

  R_xlen_t i10sz = x.size();

  Rcpp::CharacterVector mjr(i10sz);
  Rcpp::CharacterVector mnr(i10sz);
  std::string::size_type sz;

  for (R_xlen_t i = 0; i != i10sz; ++i) {
    Rcpp::String thisShort = x[i];
    if (thisShort == NA_STRING) {
      mnr[i] = NA_STRING;
      continue;
    }

    std::string s(thisShort.get_cstring()); // maybe faster to use as?
    s = strimCpp(s); // in place or rewrite? do this at all?
    sz = s.size();

    if (sz <= 3 && sz > 0) {
      mjr[i] = s.substr(0, sz);
      mnr[mnr_empty];
    } else if (sz > 3) {
      mjr[i] = s.substr(0, 3);
      mnr[i] = s.substr(3, sz - 3);
    } else {
      mjr[i] = NA_STRING;
      mnr[i] = NA_STRING;
    }
  } // for

  return icd9MajMinToParts(mjr, mnr);
}

//' @rdname convert
//' @keywords internal manip
// [[Rcpp::export]]
Rcpp::List icd9DecimalToPartsCpp(const Rcpp::CharacterVector icd9Decimal, const Rcpp::String mnr_empty) {
  Rcpp::CharacterVector mjrs;
  Rcpp::CharacterVector mnrs;
  int ilen = icd9Decimal.length();

  if (ilen == 0) {
    return Rcpp::List::create(Rcpp::_["mjr"] =
                              Rcpp::CharacterVector::create(), Rcpp::_["mnr"] =
                              Rcpp::CharacterVector::create());
  }

  for (Rcpp::CharacterVector::const_iterator it = icd9Decimal.begin();
       it != icd9Decimal.end(); ++it) {
    Rcpp::String strna = *it;
    if (strna == NA_STRING || strna == "") {
      mjrs.push_back(NA_STRING);
      mnrs.push_back(NA_STRING);
      continue;
    }
    // SOMEDAY, a faster way might be to use Rcpp::String's function
    // get_cstring, and recode the trim functions to take const char *. This
    // would avoid the type change AND may trim faster.
    std::string thiscode = Rcpp::as<std::string>(*it);
    thiscode = strimCpp(thiscode); // This updates 'thisccode' by reference, no copy
    std::size_t pos = thiscode.find(".");
    // substring parts
    std::string mjrin;
    Rcpp::String mnrout;
    if (pos != std::string::npos) {
      mjrin = thiscode.substr(0, pos);
      mnrout = thiscode.substr(pos + 1);
    } else {
      mjrin = thiscode;
      mnrout = mnr_empty;
    }
    mjrs.push_back(icd9AddLeadingZeroesMajorSingle(mjrin));
    mnrs.push_back(mnrout);
  }
  return Rcpp::List::create(Rcpp::_["mjr"] = mjrs, Rcpp::_["mnr"] =
                            mnrs);
}


//' @describeIn icd_decimal_to_parts Convert decimal ICD-10 code to parts. This
//'   shares almost 100% code with the ICD-9 version: someday combine the common
//'   code.
//' @export
//' @keywords internal manip
// [[Rcpp::export(icd_decimal_to_parts.icd10)]]
Rcpp::List icd10DecimalToPartsCpp(const Rcpp::CharacterVector x, const Rcpp::String mnr_empty = "") {
  Rcpp::CharacterVector mjrs;
  Rcpp::CharacterVector mnrs;
  R_xlen_t ilen = x.length();

  if (ilen == 0) {
    return Rcpp::List::create(Rcpp::_["mjr"] =
                              Rcpp::CharacterVector::create(), Rcpp::_["mnr"] =
                              Rcpp::CharacterVector::create());
  }

  for (Rcpp::CharacterVector::const_iterator it = x.begin();
       it != x.end(); ++it) {
    Rcpp::String strna = *it;
    if (strna == NA_STRING || strna == "") {
      mjrs.push_back(NA_STRING);
      mnrs.push_back(NA_STRING);
      continue;
    }
    std::string thiscode = Rcpp::as<std::string>(*it);
    thiscode = strimCpp(thiscode); // This updates 'thisccode' by reference, no copy
    std::size_t pos = thiscode.find(".");
    // substring parts
    std::string mjrin;
    Rcpp::String mnrout;
    if (pos != std::string::npos) {
      mjrin = thiscode.substr(0, pos);
      mnrout = thiscode.substr(pos + 1);
    } else {
      mjrin = thiscode;
      mnrout = mnr_empty;
    }
    mjrs.push_back(mjrin);
    mnrs.push_back(mnrout);
  }
  return Rcpp::List::create(Rcpp::_["mjr"] = mjrs, Rcpp::_["mnr"] =
                            mnrs);
}

// [[Rcpp::export(name = "icd9_short_to_decimal_cpp")]]
Rcpp::CharacterVector icd9ShortToDecimal(const Rcpp::CharacterVector x) {
  return icd9PartsToDecimal(icd9ShortToPartsCpp(x, ""));
}

// [[Rcpp::export(name="icd9_decimal_to_short_cpp")]]
Rcpp::CharacterVector icd9DecimalToShort(
    const Rcpp::CharacterVector x) {
  Rcpp::CharacterVector out = clone(x); // clone instead of pushing back thousands of times
  size_t ilen = x.length();
  if (ilen == 0)
    return out;
  for (size_t i = 0; i != ilen; ++i) {
    Rcpp::String strna = x[i]; // need to copy here? does it copy?
    if (strna == NA_STRING || strna == "")
      continue;
    const char * thiscode_cstr = strna.get_cstring();
    std::string thiscode(thiscode_cstr);
    thiscode = trimLeftCpp(thiscode);
    // TODO consider rejecting grossly invalid codes as NA:
    std::size_t pos = thiscode.find_first_of(".");
    if (pos != std::string::npos) {
      // now we assume that the mjr is snug against the left side, so we can add zero padding
      thiscode.erase(pos, 1); // remove the decimal point
      // could do fewer tests on the code by doing this last, but most codes are not V or E...
      if (pos > 0 && pos < 4 && !icd9IsASingleVE(thiscode_cstr)) {
        thiscode.insert(0, 3 - pos, '0');
      } else if (pos == 2 && icd9IsASingleV(thiscode_cstr)) {
        thiscode.insert(1, 1, '0');
        out[i] = thiscode;
      } else if ((pos == 2 || pos == 3) && icd9IsASingleE(thiscode_cstr)) {
        thiscode.insert(1, 4 - pos, '0');
      }
      // otherwise leave the code alone
      out[i] = thiscode;

    } else {
      out[i] = Rcpp::String(icd9AddLeadingZeroesMajorSingleStd(thiscode));
    }
  }
  return out;
}

//' @describeIn icd_get_mjr Get mjr part of ICD-9 code, i.e. first three
//' digits of numeric or V code, or first four digits of E code. This is the part
//' before the decimal, when a decimal point is used.
//' @keywords internal manip
//[[Rcpp::export(name="icd_get_mjr.icd9")]]
Rcpp::CharacterVector icd9GetMajor(const Rcpp::CharacterVector x, const bool short_code) {
  if (short_code) {
    // am I casting (or just compiler/syntax checker hinting?) SEXP may be
    // costly, or is it just encapsulating a pointer to some fixed data somewhere?

    // I don't think i need to PROTECT here, because I immediately return the
    // result via Rcpp
    SEXP mjrs = icd9ShortToPartsCpp(x, "")[0]; // actually wants to be an Rcpp::List
    return Rcpp::as<Rcpp::CharacterVector>(mjrs);
  }
  SEXP mjrs = icd9DecimalToPartsCpp(x, "")[0];
  return Rcpp::as<Rcpp::CharacterVector>(mjrs);
}
