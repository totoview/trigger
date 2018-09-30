#include <iostream>
#include "matcher.h"
#include "pred.h"
#include "var.h"


static int handler(unsigned int id, unsigned long long from, unsigned long long to, unsigned int flags, void *ctx)
{
	static_cast<Matcher*>(ctx)->onMatched(id, from, to, flags);
	return 0;
}

void Matcher::onMatched(unsigned int id, unsigned long long from, unsigned long long to, unsigned int flags)
{
	matchCnt++;
	results->push_back(predicates[id]);
	std::cout << "#### match for pattern " << id << " at offset " << to << '\n';
}

Matcher::~Matcher()
{
	if (scratch)
		hs_free_scratch(scratch);

	if (db)
		hs_free_database(db);
}

String Matcher::variable() const
{
	return var->name;
}

void Matcher::init()
{
	using pchar = const char*;
	using uint = unsigned int;

	size_t cnt = predicates.size();

	pchar* patterns = new pchar[cnt];
	uint* flags = new uint[cnt];
	uint* ids = new uint[cnt];

	for (int i = 0; i < cnt; i++) {
		patterns[i] = static_cast<PStringMatch*>(predicates[i])->patternString();
		std::cout << "[compile pattern] " << i+1 << "/" << cnt << ": " << patterns[i] << '\n';
		flags[i] = HS_FLAG_DOTALL;
		ids[i] = i;
	}

	hs_compile_error_t* err;
	if (hs_compile_multi(patterns, flags, ids, cnt, HS_MODE_BLOCK, NULL, &db, &err) != HS_SUCCESS) {
		std::string msg = std::string{"failed to compile patterns: "} + err->message;
		hs_free_compile_error(err);
		delete [] patterns;
		delete [] flags;
		delete [] ids;
		throw msg;
	}

	if (hs_alloc_scratch(db, &scratch) != HS_SUCCESS) {
		throw "failed to allocate scratch space";
	}

	delete [] patterns;
	delete [] flags;
	delete [] ids;
}

size_t Matcher::match(Vector<Predicate*>& c)
{
	const char* input = var->stringValue.c_str();
	results = &c;
	matchCnt = 0;

	std::cout << "===== match: " << var->stringValue << '\n';
	if (hs_scan(db, input, strlen(input), 0, scratch, handler, this) != HS_SUCCESS) {
		// std::cout << "failed to scan input buffer\n";
	}
	return matchCnt;
}
