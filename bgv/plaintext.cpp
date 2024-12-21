#include "plaintext.h"

namespace fheprac
{
	Plaintext::Plaintext() : slot_count_(0) {}

	Plaintext::Plaintext(uint64_t slot_count) : slot_count_(slot_count)
	{
		slots_.assign(slot_count_, 0);
	}

	int64_t& Plaintext::operator[](uint64_t index)
	{
		// TODO: ∫§≈Õ ¿Œµ¶Ω∫ øπø‹√≥∏Æ
		return slots_[index];
	}

	void Plaintext::assign(uint64_t slot_count)
	{
		slot_count_ = slot_count;
		slots_.assign(slot_count_, 0);
	}

	uint64_t Plaintext::slot_count() const
	{
		return slot_count_;
	}
}