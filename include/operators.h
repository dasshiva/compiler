#ifndef __OPERATORS_H__
#define __OPERATORS_H__

/* The following requirements must be true before adding elements here
 * - All elements of this eneum must be named with: OP_{ARITY}_{NAME}
 * - The operators in this enum must be ordered in ascending order of arity,
 *    with unary coming first, binary second and so on. 
 * - Operators of the same arity can appear in any order.
 * - Make sure that you always add an operator before the OP_{ARITY}_MAX 
 *   for the arity of your operator
 */

typedef enum OperatorCode {
	OP_UNARY_ADD, 
	OP_UNARY_SUB,
	OP_UNARY_MAX,

	OP_BINARY_ADD, 
	OP_BINARY_SUB,
	OP_BINARY_MUL, 
	OP_BINARY_DIV, 
	OP_BINARY_EQUALS, 
	OP_BINARY_MOD,
	OP_BINARY_MAX,

	OP_MAX
} OperatorCode;

int OperatorArity(OperatorCode code);
const char* Operator2String(OperatorCode code);

#endif
