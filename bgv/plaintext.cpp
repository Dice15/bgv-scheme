#include "plaintext.h"

namespace fheprac
{
	Plaintext::Plaintext(PolyMatrix poly_matrix) : pt_(poly_matrix) {}

	PolyMatrix& Plaintext::data()
	{
		return pt_;
	}
}