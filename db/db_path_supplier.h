
#include "db/column_family.h"
#include "rocksdb/status.h"

namespace rocksdb {

// An object vendored by column family to dynamically supply db path to
// functions that need to decide which db_path to flush an sst file to. The
// supplier object is mutable (because you can add file size to it) and
// can update global file counters, so usage should be inside proper locking.
class DbPathSupplier {
 public:
  virtual ~DbPathSupplier() = default;

  virtual uint32_t GetPathId(int level) const;

  virtual void AddFileSize(uint64_t file_size);

  virtual Status FsyncDbPath(uint32_t path_id) const;

  // Is the given path_id an acceptable path_id
  // for this supplier?
  //
  // This method is used in compaction to decide
  // if it is feasible to only change the level
  // of an sst file without actually moving the
  // data (if it's a trivial move).
  //
  // For a random path supplier, for example,
  // it doesn't matter which path_id is given
  // because path_ids are chosen randomly anyway.
  // For a fix path supplier, however, the given
  // path_id really needs to match the fixed
  // path_id in order for us to say it's trivial.
  virtual bool AcceptPathId(uint32_t path_id) const;
};

class FixedDbPathSupplier: DbPathSupplier {
 public:
  FixedDbPathSupplier(uint32_t path_id, Directory* path_dir)
    :path_id_(path_id), path_dir_(path_dir) {}

  uint32_t GetPathId(int level) const override {
    return path_id_;
  }

  void AddFileSize(uint64_t file_size) override {}

  Status FsyncDbPath(uint32_t path_id) const override {
    if (path_dir_ == nullptr) {
      return Status::InvalidArgument("path_dir_ is null");
    }

    return path_dir_->Fsync();
  }

  bool AcceptPathId(uint32_t path_id) const override {
    return path_id == path_id_;
  }

 private:
  uint32_t path_id_;
  Directory* path_dir_;
};

class RandomDbPathSupplier: DbPathSupplier {
 public:
  RandomDbPathSupplier(ColumnFamilyData* cfd)
    :cfd_(cfd) {}

  uint32_t GetPathId(int level) const override;

  void AddFileSize(uint64_t file_size) override {}

  Status FsyncDbPath(uint32_t path_id) const override;

  bool AcceptPathId(uint32_t path_id) const override;

 private:
  ColumnFamilyData* cfd_;
};

}
