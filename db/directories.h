#include "rocksdb/env.h"
#include "rocksdb/options.h"

namespace rocksdb {

// Class to maintain directories for all database paths other than main one.
class Directories {
 public:
  Status SetDirectories(Env* env, const std::string& dbname,
  const std::string& wal_dir,
  const std::vector<DbPath>& data_paths);

  Directory* GetDataDir(size_t path_id) const {
    assert(path_id < data_dirs_.size());
    Directory* ret_dir = data_dirs_[path_id].get();
    if (ret_dir == nullptr) {
      // Should use db_dir_
      return db_dir_.get();
    }
    return ret_dir;
  }

  Directory* GetWalDir() const {
    if (wal_dir_) {
      return wal_dir_.get();
    }
    return db_dir_.get();
  }

  Directory* GetDbDir() const { return db_dir_.get(); }

  static Status CreateAndNewDirectory(Env* env, const std::string& dirname,
      std::unique_ptr<Directory>* directory);

 private:
  std::unique_ptr<Directory> db_dir_;
  std::vector<std::unique_ptr<Directory>> data_dirs_;
  std::unique_ptr<Directory> wal_dir_;
};

}
