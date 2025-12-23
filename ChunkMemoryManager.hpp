#ifndef CHUNK_MEMORY_MANAGER_HPP
#define CHUNK_MEMORY_MANAGER_HPP

#include <cstdint>
#include <cstddef>
#include <memory>
#include <vector>

namespace cmm {

class ChunkMemoryManager {
	public:
		ChunkMemoryManager(size_t nmemb, size_t size);
		~ChunkMemoryManager(void);

		void * alloc(size_t size);
		void free(void *position);

		static void start(size_t nmemb, size_t size);
		static ChunkMemoryManager& get(void);

	private:
		uint8_t *data;
		size_t total_size;
		size_t chunk_size;
		size_t top;

		std::vector<void *> free_position;

		static std::unique_ptr<ChunkMemoryManager> global_memory_manager;
};

/**
 * Iniciar o alocador de memória.
 * nmemb = Número de membros.
 * size = Tamanho do chunk.
 */
void start(size_t nmemb, size_t size);

template<typename T>
struct PointerDeleter {
	inline void operator()(T* p) const noexcept {
		p->~T();
		ChunkMemoryManager::get().free((void *) p);
	}
};

/**
 * Equivalente ao std::unique_ptr, mas utiliza
 * o alocador customizado.
 */
template<typename T>
using unique_ptr = std::unique_ptr<T, PointerDeleter<T>>;

/**
 * Equivalente ao std::make_unique, mas utiliza o alocador
 * customizado.
 */
template<typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args) {
	void *raw_mem = ChunkMemoryManager::get().alloc(sizeof(T));

	if(raw_mem == nullptr) {
		return nullptr;
	}

	T *ptr = new (raw_mem) T(std::forward<Args>(args)...);

	return unique_ptr<T>(ptr);
}

}

#ifdef CHUNK_MEMORY_MANAGER_IMPLEMENTATION

namespace cmm {

std::unique_ptr<ChunkMemoryManager> ChunkMemoryManager::global_memory_manager = nullptr;

ChunkMemoryManager::ChunkMemoryManager(size_t nmemb, size_t size) :
	data(new uint8_t[size * nmemb]),
	total_size(size * nmemb),
	chunk_size(size),
	top(0)
{
}

ChunkMemoryManager::~ChunkMemoryManager(void) {
	delete data;
}

void * ChunkMemoryManager::alloc(size_t size) {
	void *return_position;

	if(size > chunk_size) {
		return nullptr;
	}

	if(!free_position.empty()) {
		return_position = free_position.back();
		free_position.pop_back();

		return return_position;
	}

	if(top + chunk_size >= total_size) {
		return nullptr;
	}

	return_position = static_cast<void *>(data + top);

	top += chunk_size;

	return return_position;
}

void ChunkMemoryManager::free(void *position) {
	free_position.push_back(position);
}

void ChunkMemoryManager::start(size_t nmemb, size_t size) {
	global_memory_manager = std::make_unique<ChunkMemoryManager>(nmemb, size);
}

ChunkMemoryManager& ChunkMemoryManager::get(void) {
	return *global_memory_manager;
}

void start(size_t nmemb, size_t size) {
	ChunkMemoryManager::start(nmemb, size);
}

}

#endif

#endif
