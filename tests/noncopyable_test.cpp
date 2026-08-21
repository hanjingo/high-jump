#include <gtest/gtest.h>
#include <hj/types/noncopyable.hpp>
#include <type_traits>

class DerivedNoncopyable : public hj::noncopyable
{
  public:
    DerivedNoncopyable() = default;
};

static_assert(std::is_default_constructible_v<DerivedNoncopyable>,
              "Should be default constructible");
static_assert(std::is_destructible_v<DerivedNoncopyable>,
              "Should be destructible");
static_assert(!std::is_copy_constructible_v<DerivedNoncopyable>,
              "Should NOT be copy constructible");
static_assert(!std::is_copy_assignable_v<DerivedNoncopyable>,
              "Should NOT be copy assignable");
static_assert(!std::is_move_constructible_v<DerivedNoncopyable>,
              "Should NOT be move constructible");
static_assert(!std::is_move_assignable_v<DerivedNoncopyable>,
              "Should NOT be move assignable");

class CopyDisabled
{
  public:
    CopyDisabled()                                    = default;
    CopyDisabled(CopyDisabled &&) noexcept            = default;
    CopyDisabled &operator=(CopyDisabled &&) noexcept = default;

  private:
    HJ_DISABLE_COPY(CopyDisabled)
};

static_assert(std::is_default_constructible_v<CopyDisabled>,
              "Should be default constructible");
static_assert(!std::is_copy_constructible_v<CopyDisabled>,
              "Should NOT be copy constructible");
static_assert(!std::is_copy_assignable_v<CopyDisabled>,
              "Should NOT be copy assignable");
static_assert(std::is_move_constructible_v<CopyDisabled>,
              "Should be move constructible");
static_assert(std::is_move_assignable_v<CopyDisabled>,
              "Should be move assignable");

class MoveDisabled
{
  public:
    MoveDisabled()                                = default;
    MoveDisabled(const MoveDisabled &)            = default;
    MoveDisabled &operator=(const MoveDisabled &) = default;

  private:
    HJ_DISABLE_MOVE(MoveDisabled)
};

static_assert(std::is_default_constructible_v<MoveDisabled>,
              "Should be default constructible");
static_assert(std::is_copy_constructible_v<MoveDisabled>,
              "Should be copy constructible");
static_assert(std::is_copy_assignable_v<MoveDisabled>,
              "Should be copy assignable");
static_assert(!std::is_move_constructible_v<MoveDisabled>,
              "Should NOT be move constructible");
static_assert(!std::is_move_assignable_v<MoveDisabled>,
              "Should NOT be move assignable");

class CopyMoveDisabled
{
  public:
    CopyMoveDisabled() = default;

  private:
    HJ_DISABLE_COPY_MOVE(CopyMoveDisabled)
};

static_assert(std::is_default_constructible_v<CopyMoveDisabled>,
              "Should be default constructible");
static_assert(!std::is_copy_constructible_v<CopyMoveDisabled>,
              "Should NOT be copy constructible");
static_assert(!std::is_copy_assignable_v<CopyMoveDisabled>,
              "Should NOT be copy assignable");
static_assert(!std::is_move_constructible_v<CopyMoveDisabled>,
              "Should NOT be move constructible");
static_assert(!std::is_move_assignable_v<CopyMoveDisabled>,
              "Should NOT be move assignable");

TEST(NoncopyableMatrixTest, RuntimeSanity)
{
    DerivedNoncopyable d;
    CopyDisabled       cd;
    MoveDisabled       md;
    CopyMoveDisabled   cmd;
    SUCCEED();
}