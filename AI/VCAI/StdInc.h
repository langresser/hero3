#pragma  once
#include "../../Global.h"
#include <cassert>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/type_traits.hpp>

#include <boost/foreach.hpp>
#include <boost/thread/tss.hpp>

#include "Fuzzy.h"

#include <fstream>
#include <queue>

using boost::format;
using boost::str;

extern CLogger &aiLogger;

#define INDENT AILogger::Tab ___dummy_ind
#define PNLOG(txt) {int i = logger.lvl; while(i--) aiLogger << "\t"; aiLogger << txt << "\n";}
#define BNLOG(txt, formattingEls) {int i = logger.lvl; while(i--) aiLogger << "\t"; aiLogger << (boost::format(txt) % formattingEls) << "\n";}
//#define LOG_ENTRY PNLOG("Entered " __FUNCTION__)
#define LOG_ENTRY

