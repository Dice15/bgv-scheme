#pragma once

#include "context.h"

namespace fheprac
{
	class Plaintext
	{
	public:
		Plaintext();

		Plaintext(uint64_t slot_count);

		int64_t& operator[](uint64_t index);

		void assign(uint64_t slot_count);

		uint64_t slot_count() const;

	private:
		uint64_t slot_count_;

		std::vector<int64_t> slots_;
	};
}