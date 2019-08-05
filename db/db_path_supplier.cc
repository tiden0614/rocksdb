#include <chrono>
#include "db/db_path_supplier.h"

using namespace std::chrono;
namespace rocksdb {

size_t get_cf_path_size(ColumnFamilyData* cfd) {
  size_t paths_size = cfd->ioptions()->cf_paths.size();

  if (paths_size == 0) {
    paths_size = cfd->ioptions()->db_paths.size();
  }

  return paths_size;
}

Random init_random() {
  milliseconds ms = duration_cast<milliseconds>(
      system_clock::now().time_since_epoch());
  auto seed = static_cast<uint32_t>(ms.count());
  return Random(seed);
}

uint32_t RandomDbPathSupplier::GetPathId(int level) const {
  thread_local Random rand = init_random();
  size_t paths_size = get_cf_path_size(cfd_);
  auto size_u32 = static_cast<uint32_t >(paths_size);
  return rand.Next() % size_u32;
}

Status RandomDbPathSupplier::FsyncDbPath(uint32_t path_id) const {
  Directory* dir = cfd_->GetCfAndDbDir(path_id);

  if (dir == nullptr) {
    return Status::InvalidArgument("no dir to sync for path_id "
      + std::to_string(path_id));
  }

  return dir->Fsync();
}

bool RandomDbPathSupplier::AcceptPathId(uint32_t path_id) const {
  size_t paths_size = get_cf_path_size(cfd_);

  if (paths_size == 0) {
    return path_id == 0;
  }

  auto size_u32 = static_cast<uint32_t>(paths_size);
  return path_id < size_u32;
}

}
