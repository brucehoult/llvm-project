//===- llvm/Support/FileSystem.h - File System OS Concept -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the llvm::sys::fs namespace. It is designed after
// TR2/boost filesystem (v3), but modified to remove exception handling and the
// path class.
//
// All functions return an error_code and their actual work via the last out
// argument. The out argument is defined if and only if errc::success is
// returned. A function may return any error code in the generic or system
// category. However, they shall be equivalent to any error conditions listed
// in each functions respective documentation if the condition applies. [ note:
// this does not guarantee that error_code will be in the set of explicitly
// listed codes, but it does guarantee that if any of the explicitly listed
// errors occur, the correct error_code will be used ]. All functions may
// return errc::not_enough_memory if there is not enough memory to complete the
// operation.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_FILE_SYSTEM_H
#define LLVM_SUPPORT_FILE_SYSTEM_H

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/system_error.h"
#include <ctime>
#include <iterator>
#include <string>

namespace llvm {
namespace sys {
namespace fs {

/// file_type - An "enum class" enumeration for the file system's view of the
///             type.
struct file_type {
  enum _ {
    status_error,
    file_not_found,
    regular_file,
    directory_file,
    symlink_file,
    block_file,
    character_file,
    fifo_file,
    socket_file,
    type_unknown
  };

  file_type(_ v) : v_(v) {}
  explicit file_type(int v) : v_(_(v)) {}
  operator int() const {return v_;}

private:
  int v_;
};

/// copy_option - An "enum class" enumeration of copy semantics for copy
///               operations.
struct copy_option {
  enum _ {
    fail_if_exists,
    overwrite_if_exists
  };

  copy_option(_ v) : v_(v) {}
  explicit copy_option(int v) : v_(_(v)) {}
  operator int() const {return v_;}

private:
  int v_;
};

/// space_info - Self explanatory.
struct space_info {
  uint64_t capacity;
  uint64_t free;
  uint64_t available;
};

/// file_status - Represents the result of a call to stat and friends. It has
///               a platform specific member to store the result.
class file_status
{
  // implementation defined status field.
  file_type Type;
public:
  explicit file_status(file_type v=file_type::status_error)
    : Type(v) {}

  file_type type() const { return Type; }
  void type(file_type v) { Type = v; }
};

/// @}
/// @name Physical Operators
/// @{

/// @brief Copy the file at \a from to the path \a to.
///
/// @param from The path to copy the file from.
/// @param to The path to copy the file to.
/// @param copt Behavior if \a to already exists.
/// @returns errc::success if the file has been successfully copied.
///          errc::file_exists if \a to already exists and \a copt ==
///          copy_option::fail_if_exists. Otherwise a platform specific
///          error_code.
error_code copy_file(const Twine &from, const Twine &to,
                     copy_option copt = copy_option::fail_if_exists);

/// @brief Create all the non-existent directories in path.
///
/// @param path Directories to create.
/// @param existed Set to true if \a path already existed, false otherwise.
/// @returns errc::success if is_directory(path) and existed have been set,
///          otherwise a platform specific error_code.
error_code create_directories(const Twine &path, bool &existed);

/// @brief Create the directory in path.
///
/// @param path Directory to create.
/// @param existed Set to true if \a path already existed, false otherwise.
/// @returns errc::success if is_directory(path) and existed have been set,
///          otherwise a platform specific error_code.
error_code create_directory(const Twine &path, bool &existed);

/// @brief Create a hard link from \a from to \a to.
///
/// @param to The path to hard link to.
/// @param from The path to hard link from. This is created.
/// @returns errc::success if exists(to) && exists(from) && equivalent(to, from)
///          , otherwise a platform specific error_code.
error_code create_hard_link(const Twine &to, const Twine &from);

/// @brief Create a symbolic link from \a from to \a to.
///
/// @param to The path to symbolically link to.
/// @param from The path to symbolically link from. This is created.
/// @returns errc::success if exists(to) && exists(from) && is_symlink(from),
///          otherwise a platform specific error_code.
error_code create_symlink(const Twine &to, const Twine &from);

/// @brief Remove path. Equivalent to POSIX remove().
///
/// @param path Input path.
/// @param existed Set to true if \a path existed, false if it did not.
///                undefined otherwise.
/// @results errc::success if path has been removed and existed has been
///          successfully set, otherwise a platform specific error_code.
error_code remove(const Twine &path, bool &existed);

/// @brief Recursively remove all files below \a path, then \a path. Files are
///        removed as if by POSIX remove().
///
/// @param path Input path.
/// @param num_removed Number of files removed.
/// @results errc::success if path has been removed and num_removed has been
///          successfully set, otherwise a platform specific error_code.
error_code remove_all(const Twine &path, uint32_t &num_removed);

/// @brief Rename \a from to \a to. Files are renamed as if by POSIX rename().
///
/// @param from The path to rename from.
/// @param to The path to rename to. This is created.
error_code rename(const Twine &from, const Twine &to);

/// @brief Resize path to size. File is resized as if by POSIX truncate().
///
/// @param path Input path.
/// @param size Size to resize to.
/// @returns errc::success if \a path has been resized to \a size, otherwise a
///          platform specific error_code.
error_code resize_file(const Twine &path, uint64_t size);

/// @brief Make file readable.
///
/// @param path Input path.
/// @param value If true, make readable, else, make unreadable.
/// @results errc::success if readability has been successfully set, otherwise a
///          platform specific error_code.
error_code set_read(const Twine &path, bool value);

/// @brief Make file writeable.
///
/// @param path Input path.
/// @param value If true, make writeable, else, make unwriteable.
/// @results errc::success if writeability has been successfully set, otherwise
///          a platform specific error_code.
error_code set_write(const Twine &path, bool value);

/// @brief Make file executable.
///
/// @param path Input path.
/// @param value If true, make executable, else, make unexecutable.
/// @results errc::success if executability has been successfully set, otherwise
///          a platform specific error_code.
error_code set_execute(const Twine &path, bool value);

/// @}
/// @name Physical Observers
/// @{

/// @brief Does file exist?
///
/// @param status A file_status previously returned from stat.
/// @param result Set to true if the file represented by status exists, false if
///               it does not. Undefined otherwise.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code exists(file_status status, bool &result);

/// @brief Does file exist?
///
/// @param path Input path.
/// @param result Set to true if the file represented by status exists, false if
///               it does not. Undefined otherwise.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code exists(const Twine &path, bool &result);

/// @brief Do paths represent the same thing?
///
/// @param A Input path A.
/// @param B Input path B.
/// @param result Set to true if stat(A) and stat(B) have the same device and
///               inode (or equivalent).
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code equivalent(const Twine &A, const Twine &B, bool &result);

/// @brief Get file size.
///
/// @param path Input path.
/// @param result Set to the size of the file in \a path.
/// @returns errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code file_size(const Twine &path, uint64_t &result);

/// @brief Does status represent a directory?
///
/// @param status A file_status previously returned from stat.
/// @param result Set to true if the file represented by status is a directory,
///               false if it is not. Undefined otherwise.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code is_directory(file_status status, bool &result);

/// @brief Is path a directory?
///
/// @param path Input path.
/// @param result Set to true if \a path is a directory, false if it is not.
///               Undefined otherwise.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code is_directory(const Twine &path, bool &result);

/// @brief Is path an empty file?
///
/// @param path Input path.
/// @param result Set to true if \a path is a an empty file, false if it is not.
///               Undefined otherwise.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code is_empty(const Twine &path, bool &result);

/// @brief Does status represent a regular file?
///
/// @param status A file_status previously returned from stat.
/// @param result Set to true if the file represented by status is a regular
///               file, false if it is not. Undefined otherwise.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code is_regular_file(file_status status, bool &result);

/// @brief Is path a regular file?
///
/// @param path Input path.
/// @param result Set to true if \a path is a regular file, false if it is not.
///               Undefined otherwise.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code is_regular_file(const Twine &path, bool &result);

/// @brief Does status represent something that exists but is not a directory,
///        regular file, or symlink?
///
/// @param status A file_status previously returned from stat.
/// @param result Set to true if the file represented by status exists, but is
///               not a directory, regular file, or a symlink, false if it does
///               not. Undefined otherwise.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code is_other(file_status status, bool &result);

/// @brief Is path something that exists but is not a directory,
///        regular file, or symlink?
///
/// @param path Input path.
/// @param result Set to true if \a path exists, but is not a directory, regular
///               file, or a symlink, false if it does not. Undefined otherwise.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code is_other(const Twine &path, bool &result);

/// @brief Does status represent a symlink?
///
/// @param status A file_status previously returned from stat.
/// @param result Set to true if the file represented by status is a symlink,
///               false if it is not. Undefined otherwise.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code is_symlink(file_status status, bool &result);

/// @brief Is path a symlink?
///
/// @param path Input path.
/// @param result Set to true if \a path is a symlink, false if it is not.
///               Undefined otherwise.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code is_symlink(const Twine &path, bool &result);

/// @brief Get last write time without changing it.
///
/// @param path Input path.
/// @param result Set to the last write time (UNIX time) of \a path if it
///               exists.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code last_write_time(const Twine &path, std::time_t &result);

/// @brief Set last write time.
///
/// @param path Input path.
/// @param value Time to set (UNIX time) \a path's last write time to.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code set_last_write_time(const Twine &path, std::time_t value);

/// @brief Read a symlink's value.
///
/// @param path Input path.
/// @param result Set to the value of the symbolic link \a path.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code read_symlink(const Twine &path, SmallVectorImpl<char> &result);

/// @brief Get disk space usage information.
///
/// @param path Input path.
/// @param result Set to the capacity, free, and available space on the device
///               \a path is on.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code disk_space(const Twine &path, space_info &result);

/// @brief Get file status as if by POSIX stat().
///
/// @param path Input path.
/// @param result Set to the file status.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code status(const Twine &path, file_status &result);

/// @brief Is status available?
///
/// @param path Input path.
/// @param result Set to true if status() != status_error.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code status_known(const Twine &path, bool &result);

/// @brief Get file status as if by POSIX lstat().
///
/// Does not resolve symlinks.
///
/// @param path Input path.
/// @param result Set to the file status.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code symlink_status(const Twine &path, file_status &result);

/// @brief Generate a unique path and open it as a file.
///
/// Generates a unique path suitable for a temporary file and then opens it as a
/// file. The name is based on \a model with '%' replaced by a random char in
/// [0-9a-f]. If \a model is not an absolute path, a suitable temporary
/// directory will be prepended.
///
/// This is an atomic operation. Either the file is created and opened, or the
/// file system is left untouched.
///
/// clang-%%-%%-%%-%%-%%.s => /tmp/clang-a0-b1-c2-d3-e4.s
///
/// @param model Name to base unique path off of.
/// @param result_fs Set to the opened file's file descriptor.
/// @param result_path Set to the opened file's absolute path.
/// @results errc::success if result_{fd,path} have been successfully set,
///          otherwise a platform specific error_code.
error_code unique_file(const Twine &model, int &result_fd,
                             SmallVectorImpl<char> &result_path);

/// @brief Canonicalize path.
///
/// Sets result to the file system's idea of what path is. The result is always
/// absolute and has the same capitalization as the file system.
///
/// @param path Input path.
/// @param result Set to the canonicalized version of \a path.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code canonicalize(const Twine &path, SmallVectorImpl<char> &result);

/// @brief Are \a path's first bytes \a magic?
///
/// @param path Input path.
/// @param magic Byte sequence to compare \a path's first len(magic) bytes to.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code has_magic(const Twine &path, const Twine &magic);

/// @brief Get \a path's first \a len bytes.
///
/// @param path Input path.
/// @param len Number of magic bytes to get.
/// @param result Set to the first \a len bytes in the file pointed to by
///               \a path.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code get_magic(const Twine &path, uint32_t len,
                     SmallVectorImpl<char> &result);

/// @brief Is file bitcode?
///
/// @param path Input path.
/// @param result Set to true if \a path is a bitcode file, false if it is not,
///               undefined otherwise.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code is_bitcode(const Twine &path, bool &result);

/// @brief Is file a dynamic library?
///
/// @param path Input path.
/// @param result Set to true if \a path is a dynamic library, false if it is
///               not, undefined otherwise.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code is_dynamic_library(const Twine &path, bool &result);

/// @brief Is an object file?
///
/// @param path Input path.
/// @param result Set to true if \a path is an object file, false if it is not,
///               undefined otherwise.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code is_object_file(const Twine &path, bool &result);

/// @brief Can file be read?
///
/// @param path Input path.
/// @param result Set to true if \a path is readable, false it it is not,
///               undefined otherwise.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code can_read(const Twine &path, bool &result);

/// @brief Can file be written?
///
/// @param path Input path.
/// @param result Set to true if \a path is writeable, false it it is not,
///               undefined otherwise.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code can_write(const Twine &path, bool &result);

/// @brief Can file be executed?
///
/// @param path Input path.
/// @param result Set to true if \a path is executable, false it it is not,
///               undefined otherwise.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code can_execute(const Twine &path, bool &result);

/// @brief Get library paths the system linker uses.
///
/// @param result Set to the list of system library paths.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code GetSystemLibraryPaths(SmallVectorImpl<std::string> &result);

/// @brief Get bitcode library paths the system linker uses
///        + LLVM_LIB_SEARCH_PATH + LLVM_LIBDIR.
///
/// @param result Set to the list of bitcode library paths.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code GetBitcodeLibraryPaths(SmallVectorImpl<std::string> &result);

/// @brief Find a library.
///
/// Find the path to a library using its short name. Use the system
/// dependent library paths to locate the library.
///
/// c => /usr/lib/libc.so
///
/// @param short_name Library name one would give to the system linker.
/// @param result Set to the absolute path \a short_name represents.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code FindLibrary(const Twine &short_name, SmallVectorImpl<char> &result);

/// @brief Get absolute path of main executable.
///
/// @param argv0 The program name as it was spelled on the command line.
/// @param MainAddr Address of some symbol in the executable (not in a library).
/// @param result Set to the absolute path of the current executable.
/// @results errc::success if result has been successfully set, otherwise a
///          platform specific error_code.
error_code GetMainExecutable(const char *argv0, void *MainAddr,
                             SmallVectorImpl<char> &result);

/// @}
/// @name Iterators
/// @{

/// directory_entry - A single entry in a directory. Caches the status either
/// from the result of the iteration syscall, or the first time status or
/// symlink_status is called.
class directory_entry {
  std::string Path;
  mutable file_status Status;
  mutable file_status SymlinkStatus;

public:
  explicit directory_entry(const Twine &path, file_status st = file_status(),
                                       file_status symlink_st = file_status());

  void assign(const Twine &path, file_status st = file_status(),
                          file_status symlink_st = file_status());
  void replace_filename(const Twine &filename, file_status st = file_status(),
                              file_status symlink_st = file_status());

  const SmallVectorImpl<char> &path() const;
  error_code status(file_status &result) const;
  error_code symlink_status(file_status &result) const;

  bool operator==(const directory_entry& rhs) const;
  bool operator!=(const directory_entry& rhs) const;
  bool operator< (const directory_entry& rhs) const;
  bool operator<=(const directory_entry& rhs) const;
  bool operator> (const directory_entry& rhs) const;
  bool operator>=(const directory_entry& rhs) const;
};

/// directory_iterator - Iterates through the entries in path. There is no
/// operator++ because we need an error_code. If it's really needed we can make
/// it call report_fatal_error on error.
class directory_iterator {
  // implementation directory iterator status

public:
  explicit directory_iterator(const Twine &path, error_code &ec);
  // No operator++ because we need error_code.
  directory_iterator &increment(error_code &ec);

  const directory_entry &operator*() const;
  const directory_entry *operator->() const;

  // Other members as required by
  // C++ Std, 24.1.1 Input iterators [input.iterators]
};

/// recursive_directory_iterator - Same as directory_iterator except for it
/// recurses down into child directories.
class recursive_directory_iterator {
  uint16_t  Level;
  bool HasNoPushRequest;
  // implementation directory iterator status

public:
  explicit recursive_directory_iterator(const Twine &path, error_code &ec);
  // No operator++ because we need error_code.
  directory_iterator &increment(error_code &ec);

  const directory_entry &operator*() const;
  const directory_entry *operator->() const;

  // observers
  /// Gets the current level. path is at level 0.
  int level() const;
  /// Returns true if no_push has been called for this directory_entry.
  bool no_push_request() const;

  // modifiers
  /// Goes up one level if Level > 0.
  void pop();
  /// Does not go down into the current directory_entry.
  void no_push();

  // Other members as required by
  // C++ Std, 24.1.1 Input iterators [input.iterators]
};

/// @}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

#endif
