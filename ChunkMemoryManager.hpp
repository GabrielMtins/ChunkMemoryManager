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

		template<typename T>
		struct GenericPointerDeleter {
			ChunkMemoryManager *memory_manager;

			inline explicit GenericPointerDeleter(ChunkMemoryManager *memory_manager = nullptr) noexcept :
				memory_manager(memory_manager)
			{
			}
		
			inline void operator()(T* p) const noexcept {
				p->~T();
				memory_manager->free(static_cast<void *>(p));
			}
		};

		/**
		 * Equivale ao std::make_unique.
		 */
		template<typename T, typename... Args>
		std::unique_ptr<T, GenericPointerDeleter<T>> make_unique(Args&&... args) {
			void *raw_mem = alloc(sizeof(T));
		
			if(raw_mem == nullptr) {
				return nullptr;
			}
		
			T *ptr = new (raw_mem) T(std::forward<Args>(args)...);
		
			return std::unique_ptr<T, GenericPointerDeleter<T>>(ptr, GenericPointerDeleter<T>(this));
		}

		/**
		 * Equivale ao std::make_shared.
		 */
		template<typename T, typename... Args>
		std::shared_ptr<T> make_shared(Args&&... args) {
			void *raw_mem = alloc(sizeof(T));

			if(raw_mem == nullptr) {
				return nullptr;
			}

			T *ptr = new (raw_mem) T(std::forward<Args>(args)...);

			return std::shared_ptr<T>(ptr, GenericPointerDeleter<T>(this));
		}

	private:
		uint8_t *data;
		size_t total_size;
		size_t chunk_size;

		std::vector<void *> free_position;
};

/**
 * Equivalente ao std::unique_ptr, mas utiliza
 * o alocador customizado.
 */
template<typename T>
using unique_ptr = std::unique_ptr<T, ChunkMemoryManager::GenericPointerDeleter<T>>;

template<typename T>
using shared_ptr = std::shared_ptr<T>;

}

#ifdef CHUNK_MEMORY_MANAGER_IMPLEMENTATION

namespace cmm {

ChunkMemoryManager::ChunkMemoryManager(size_t nmemb, size_t size) :
	data(new uint8_t[size * nmemb]),
	total_size(size * nmemb),
	chunk_size(size)
{
	free_position.reserve(nmemb);

	for(size_t i = 0; i < nmemb; i++) {
		free_position.push_back(
				static_cast<void *>(data + i * size)
				);
	}
}

ChunkMemoryManager::~ChunkMemoryManager(void) {
	delete data;
}

void * ChunkMemoryManager::alloc(size_t size) {
	if(size > chunk_size) {
		return nullptr;
	}

	if(!free_position.empty()) {
		void *return_position = free_position.back();
		free_position.pop_back();

		return return_position;
	}

	return nullptr;
}

void ChunkMemoryManager::free(void *position) {
	free_position.push_back(position);
}

}

#endif

#endif
