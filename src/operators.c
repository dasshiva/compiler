#include "operators.h"

int OperatorArity(OperatorCode code) {
	if (code < OP_UNARY_MAX)
		return 1;

	if (code > OP_UNARY_MAX && code < OP_BINARY_MAX)
		return 2;

	return -1;
}

static const char* O2S[] = {
	"u+", "u-", "unary_max_invalid", 
	"+", "-", "*", "/", "=", "%", "binary_max_invalid",
	"op_max_invalid"
};

const char* Operator2String(OperatorCode code) {
	return O2S[code];
}


