#pragma once

#include "../context.h"
#include "../encryptionparams.h"
#include "polynomial.h"
#include <stdint.h>

namespace fheprac
{
	Polynomial sample_poly_from_gaussian_dist(const Context& context, const EncryptionParameters& params);

	Polynomial sample_poly_from_uniform_dist(const Context& context, const EncryptionParameters& params);
}