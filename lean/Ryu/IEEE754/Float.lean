/-
Copyright (c) 2026 Ulf Adams. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE-Apache2.
Authors: Ulf Adams
-/
import Mathlib

namespace Ryu.IEEE754.Float

/-! # IEEE 754 Floating Point constructs

This file provides a number of constructs to model IEEE 754 Floating Point
numbers; it supports a mathematical construct F(c) to represent these numbers
in the form `(-1)^s * m * 2^e` and also allows round-tripping to a bit vector
representation.

In early 2026, I investigated the existing `Mathlib/Data/FP/Basic.lean`
definitions, but found multiple problems with them:
- They did not seem to implement IEEE 754 semantics.
- They did not seem to work correctly (try `toRat (fromRat 1)`).
- They were completely undocumented and marked as experimental.

I therefore decided to provide my own constructs.
-/
structure Cfg where
  prec : Int     -- Mantissa bits (> 1)
  ewidth : Int   -- Exponent bits (> 1)
  prec_at_least_2 : 1 < prec
  ewidth_at_least_2 : 1 < ewidth

namespace Cfg
-- Minimum normal mantissa value; this has prec + 1 bits, and only the top bit is set.
def minNormal (c : Cfg) : Int := 2 ^ (c.prec - 1).toNat

-- Maximum normal mantissa value PLUS ONE.
def maxNormal (c : Cfg) : Int := 2 ^ c.prec.toNat

-- Total number of bits in the IEEE-754 bit vector representation:
-- 1 bit for sign, + ewidth bits for the exponent + prec-1 bits for the mantissa.
def totalBits (c : Cfg) : Nat := c.ewidth.toNat + c.prec.toNat

-- Exponent bias in the IEEE-754 bit vector representation.
def bias (c : Cfg) : Int := 2 ^ (c.ewidth - 1).toNat - 1

-- Exponent representation for +/-∞ and NaN (all exponent bits set).
def maxExponent (c : Cfg) : Int := (2 ^ c.ewidth.toNat) - 1

def emax (c : Cfg) : Int := bias c

-- Smallest (negative) representable exponent; used for denormals and the first batch of normals.
def emin (c : Cfg) : Int := 1 - c.emax
end Cfg

def ValidFinite (c : Cfg) (e : Int) (m : Int) : Prop :=
  -- Zero case:
  (e = c.emin ∧ m = 0) ∨
  -- Denormal case:
  (e = c.emin ∧ 0 < m ∧ m < c.minNormal) ∨
  -- Normal case:
  (c.emin ≤ e ∧ e ≤ c.emax ∧ c.minNormal ≤ m ∧ m < c.maxNormal)

-- Prove to Lean that ValidFinite is Decidable.
instance (c : Cfg) (e : Int) (m : Int) : Decidable (ValidFinite c e m) := by
  dsimp [ValidFinite]
  infer_instance

-- Constructor for floating point numbers, supporting a single value for NaN, +/-∞, and both
-- denormal and normal numbers; see ValidFinite for constraints on denormals and normals.
inductive F (c : Cfg) where
  | nan : F c
  | inf : (neg : Bool) → F c
  | finite : (neg : Bool) → (e : Int) → (m : Int) → ValidFinite c e m → F c
deriving DecidableEq

-- Maps a float to its IEEE 754 bit vector representation
def toBits {c : Cfg} (f : F c) : BitVec (c.totalBits) :=
  match f with
  | .nan => BitVec.ofNat c.totalBits ((c.maxExponent <<< (c.prec.toNat - 1)) + 1).toNat
  | .inf neg =>
    let s := if neg then (1 : Nat) else 0
    let ebits : Nat := c.maxExponent.toNat
    let mbits : Nat := 0
    BitVec.ofNat
      c.totalBits
      ((s <<< (c.ewidth + c.prec - 1).toNat) + (ebits <<< (c.prec - 1).toNat) + mbits)
  | .finite neg e m _ =>
    let s := if neg then (1 : Nat) else 0
    let ebits : Nat := if m < c.minNormal then 0 else (e + c.bias).toNat
    let mbits : Nat := if m < c.minNormal then m.toNat else (m - c.minNormal).toNat
    BitVec.ofNat
      c.totalBits
      ((s <<< (c.ewidth + c.prec - 1).toNat) + (ebits <<< (c.prec - 1).toNat) + mbits)

lemma maxExponent_eq_two_bias_plus_one (c : Cfg) : c.maxExponent = 2 * c.bias + 1 := by
  dsimp [Cfg.maxExponent, Cfg.bias]
  -- Goal: 2 ^ c.ewidth.toNat - 1 = 2 * (2 ^ (c.ewidth - 1).toNat - 1) + 1
  have h_exp : c.ewidth.toNat = (c.ewidth - 1).toNat + 1 := by
    simp only [Int.pred_toNat]
    rw [Nat.sub_add_cancel]
    zify
    rw [Int.toNat_of_nonneg (by linarith [c.ewidth_at_least_2])]
    linarith [c.ewidth_at_least_2]
  rw [h_exp, pow_succ]
  ring

lemma maxNormal_eq_two_minNormal (c : Cfg) : c.maxNormal = 2 * c.minNormal := by
  unfold Cfg.maxNormal Cfg.minNormal
  have h_exp : c.prec.toNat = (c.prec - 1).toNat + 1 := by
    zify
    rw [Int.toNat_of_nonneg (by linarith [c.prec_at_least_2])]
    rw [Int.toNat_of_nonneg (by linarith [c.prec_at_least_2])]
    linarith
  rw [h_exp]
  rw [pow_succ]
  linarith

lemma minNormal_sub_one_lt_minNorml (c : Cfg) : (c.minNormal - 1).toNat < c.minNormal := by
  rw [Int.toNat_of_nonneg]
  · linarith
  · dsimp [Cfg.minNormal]
    apply Int.le_sub_one_of_lt
    positivity

-- Deconstructs an IEEE-754 bit vector into a Float
def fromBits {c : Cfg} (bv : BitVec c.totalBits) : (F c) :=
  let n := bv.toNat
  let s_bit := n >>> (c.ewidth + c.prec - 1).toNat
  let e_bits := (n >>> (c.prec - 1).toNat) &&& c.maxExponent.toNat
  let m_bits := n &&& (c.minNormal - 1).toNat
  -- By cases
  if he_eq_max: e_bits = c.maxExponent then
    if m_bits = 0 then -- +/-∞
      .inf (s_bit == 1)
    else -- NaN
      .nan
  else if he: e_bits = 0 then
    if hm : m_bits = 0 then
      -- Zero
      have h : ValidFinite c c.emin 0 := by
        dsimp [ValidFinite]
        apply Or.inl
        simp
      .finite (s_bit == 1) c.emin 0 h
    else
      -- Denormal: e is fixed to emin, m is exactly m_bits
      let m := Int.ofNat m_bits
      have h : ValidFinite c c.emin m := by
        dsimp [ValidFinite]
        apply Or.inr; apply Or.inl
        refine ⟨?_, ?_, ?_⟩
        · simp
        · apply Int.ofNat_lt.mpr
          apply Nat.pos_of_ne_zero hm
        · dsimp [m, m_bits]
          apply Int.lt_of_le_of_lt (b := (c.minNormal - 1).toNat)
          · apply Int.ofNat_le.mpr
            apply Nat.and_le_right
          · exact minNormal_sub_one_lt_minNorml c
      .finite (s_bit == 1) c.emin (Int.ofNat m_bits) h
  else -- Normal: e = e_bits - bias, m = m_bits + minNormal (hidden bit)
    let e := Int.ofNat e_bits - c.bias
    let m := Int.ofNat m_bits + c.minNormal
    have h : ValidFinite c e m := by
      dsimp [ValidFinite]
      apply Or.inr; apply Or.inr
      refine ⟨?_, ?_, ?_, ?_⟩
      · dsimp [e, Cfg.emin]
        rw [Cfg.emax]
        simp only [tsub_le_iff_right, sub_add_cancel, Nat.one_le_cast]
        exact Nat.pos_of_ne_zero he
      · dsimp [e]
        rw [Cfg.emax]
        simp only [tsub_le_iff_right]
        rw [Int.le_iff_lt_add_one]
        have h_rhs : c.bias + c.bias + 1 = c.maxExponent := by
          rw [maxExponent_eq_two_bias_plus_one]
          ring
        rw [h_rhs]
        apply lt_of_le_of_ne
        · dsimp [e_bits]
          have h_max_eq : c.maxExponent = ↑c.maxExponent.toNat := by
            rw [Int.toNat_of_nonneg]
            unfold Cfg.maxExponent
            simp only [Int.sub_nonneg]
            have h_pow_pos : 1 ≤ 2 ^ c.ewidth.toNat := by
              apply Nat.one_le_pow
              norm_num
            linarith
          rw [h_max_eq]
          apply Int.ofNat_le.mpr
          apply Nat.and_le_right
        · exact he_eq_max
      · dsimp [m]
        linarith
      · dsimp [m]
        have h_m_lt : ↑m_bits < c.minNormal := by
          dsimp [m_bits]
          apply Int.lt_of_le_of_lt (b:= ↑(c.minNormal - 1).toNat)
          · apply Int.ofNat_le.mpr
            apply Nat.and_le_right
          · exact minNormal_sub_one_lt_minNorml c
        rw [maxNormal_eq_two_minNormal c]
        linarith [h_m_lt]
    .finite (s_bit == 1) e m h

def succFinitePos {c : Cfg} (e : Int) (m : Int) (_ : ValidFinite c e m) : (F c) :=
  if h1 : ValidFinite c e (m + 1) then
    (F.finite (c := c) false e (m + 1) h1)
  else
    if h2 : ValidFinite c (e + 1) (c.minNormal) then
      (F.finite (c := c) false (e + 1) (c.minNormal) h2)
    else
      (F.inf false)

def succFiniteNeg {c : Cfg} (e : Int) (m : Int) (_ : ValidFinite c e m) : (F c) :=
  if h1 : ValidFinite c e (m - 1) then
    (F.finite (c := c) true e (m - 1) h1)
  else
    if h2 : ValidFinite c (e - 1) (c.maxNormal - 1) then
      (F.finite (c := c) true (e - 1) (c.maxNormal - 1) h2)
    else
      (F.finite (c := c) false (c.emin) 0 (Or.inl ⟨rfl, rfl⟩))

lemma emax_pos (c : Cfg) : 0 < c.emax := by
  rw [Cfg.emax]
  dsimp [Cfg.bias]
  have h_pow_pos : 1 < 2 ^ (c.ewidth - 1).toNat := by
    apply Nat.one_lt_pow
    · rw [← Nat.pos_iff_ne_zero]
      zify
      rw [Int.toNat_of_nonneg]
      · linarith [c.ewidth_at_least_2]
      · linarith [c.ewidth_at_least_2]
    · norm_num
  linarith

lemma ValidFinite_max_finite (c : Cfg) : ValidFinite c c.emax (c.maxNormal - 1) := by
  dsimp [ValidFinite]
  apply Or.inr; apply Or.inr
  refine ⟨?_, ?_, ?_, ?_⟩
  · simp [Cfg.emin]
    linarith [emax_pos c]
  · linarith
  · simp only [Cfg.minNormal, Cfg.maxNormal]
    rw [Int.le_sub_one_iff]
    apply pow_lt_pow_right₀ (by norm_num : 1 < (2 : Int))
    rw [Int.toNat_lt_toNat]
    · linarith [c.prec_at_least_2]
    · linarith [c.prec_at_least_2]
  · linarith

-- Returns the next representable larger number; if the input is NaN or +∞, then that is returned
-- (e.g., we don't wrap around to -∞).
def succ {c : Cfg} : (F c) → (F c)
  -- NaN stays NaN.
  | .nan => F.nan
  -- Positive numbers
  | .inf false => (F.inf false)
  | .finite false e m v => succFinitePos e m v
  -- Negative numbers
  | .inf true => (F.finite true c.emax (c.maxNormal - 1) (ValidFinite_max_finite c))
  | .finite true e m v => succFiniteNeg e m v

def toRat {c : Cfg} : F c → Option ℚ
  | .nan => none
  | .inf true => none
  | .inf false => some (2 ^ (c.emax + 1))
  | .finite neg e m _ =>
      let s : ℚ := if neg then (-1) else 1
      some (s * (m : ℚ) * 2 ^ (e - c.prec + 1))

def toRatFinite {c : Cfg} (neg : Bool) (e : Int) (m : Int) : ℚ :=
  (if neg then -1 else 1) * (m : ℚ) * (2 : ℚ) ^ (e - c.prec + 1)

lemma toRat_finite_eq {c : Cfg} (neg : Bool) (e : Int) (m : Int) (h : ValidFinite c e m) :
  toRat (F.finite neg e m h) = some (@toRatFinite c neg e m) := by rfl

variable {c : Cfg} {e : Int} {m : Int}

lemma ValidFinite_m_lt_pow : ValidFinite c e m → m < c.maxNormal := by
  intro h
  unfold ValidFinite at h
  rcases h with hzero | hden | hnorm
  · rcases hzero with ⟨_, hm0⟩
    simp [hm0, Cfg.maxNormal]
  · rcases hden with ⟨_, _, hm_lt_pow⟩
    apply lt_of_lt_of_le hm_lt_pow
    dsimp [Cfg.minNormal, Cfg.maxNormal]
    gcongr
    · norm_num
    · apply Int.toNat_le_toNat
      linarith
  · rcases hnorm with ⟨_, _, _, hm_lt_pow⟩
    exact hm_lt_pow

#check ValidFinite_m_lt_pow

lemma ValidFinite_e_le_emax : ValidFinite c e m → e ≤ c.emax := by
  intro h
  rcases h with hzero | hden | hnorm
  · rcases hzero with ⟨he_eq_emin, _⟩
    simp [he_eq_emin]
    dsimp [Cfg.emin]
    have h_emax_pos : 1 ≤ c.emax := by
      linarith [emax_pos c]
    linarith
  · rcases hden with ⟨he_eq_emin, _, _⟩
    simp [he_eq_emin]
    dsimp [Cfg.emin]
    have h_emax_pos : 1 ≤ c.emax := by
      linarith [emax_pos c]
    linarith
  · rcases hnorm with ⟨_, he_lt_emax, _, _⟩
    exact he_lt_emax

#check ValidFinite_e_le_emax

lemma minNormal_toRat : c.minNormal = (2 : ℚ) ^ (c.prec - 1) := by
  rw [Cfg.minNormal, Int.cast_pow, Int.cast_ofNat, ← zpow_natCast]
  apply congrArg (HPow.hPow (2 : ℚ))
  apply Int.toNat_of_nonneg
  linarith [c.prec_at_least_2]

lemma maxNormal_toRat : c.maxNormal = (2 : ℚ) ^ c.prec := by
  rw [Cfg.maxNormal, Int.cast_pow, Int.cast_ofNat, ← zpow_natCast]
  apply congrArg (HPow.hPow (2 : ℚ))
  apply Int.toNat_of_nonneg
  linarith [c.prec_at_least_2]

lemma toRat_of_finite_lt_pow (h : ValidFinite c e m) :
      @toRatFinite c false e m < 2 ^ (e + 1) := by
  dsimp [toRatFinite]
  simp only [one_mul]
  -- Goal: ↑m * 2 ^ (e - c.prec + 1) < 2 ^ (e + 1)
  apply lt_of_lt_of_le (b := (2 : ℚ) ^ c.prec * (2 : ℚ) ^ (e - c.prec + 1))
  · -- Part 1: ↑m * 2 ^ (e - c.prec + 1) < 2 ^ c.prec * 2 ^ (e - c.prec + 1)
    apply mul_lt_mul_of_pos_right
    · -- m < 2^p (this uses your ValidFinite_m_lt_pow lemma)
      have h_m_lt := ValidFinite_m_lt_pow h
      have h_m_rat : (m : ℚ) < (c.maxNormal : ℚ) := by exact_mod_cast h_m_lt
      rw [maxNormal_toRat] at h_m_rat
      exact h_m_rat
    · apply zpow_pos (by norm_num)
  · -- Part 2: 2 ^ c.prec * 2 ^ (e - c.prec + 1) ≤ 2 ^ (e + 1)
    -- We prove equality, which implies 'less than or equal'
    apply le_of_eq
    rw [← zpow_add₀ (by norm_num)]
    apply (congrArg (fun x : ℤ => (2 : ℚ) ^ x))
    linarith

#check toRat_of_finite_lt_pow

lemma succFinitePos_increases {c : Cfg} (h : ValidFinite c e m) (y : F c) :
  succFinitePos e m h = some y → toRat (F.finite false e m h) < toRat y := by
    intro hs
    dsimp [succFinitePos] at hs
    split at hs
    · injection hs with hy
      subst hy
      dsimp [toRat]
      simp only [one_mul, Int.cast_add, Int.cast_one, Option.some_lt_some]
      set k : Int := e - c.prec + 1
      have hk : 0 < (2:ℚ) ^ k := zpow_pos (by norm_num) k
      have hm : (m:ℚ) < m + 1 := by norm_num
      exact mul_lt_mul_of_pos_right hm hk
    · split at hs
      · -- Case 2: Mantissa overflows (m + 1 = maxNormal)
        injection hs with hy; subst hy
        rw [toRat_finite_eq _ _ _ h, toRat_finite_eq] -- Clean up both sides
        simp only [Option.some_lt_some]
        apply lt_of_lt_of_le (toRat_of_finite_lt_pow h)
        apply le_of_eq
        symm
        dsimp [toRatFinite]
        rw [minNormal_toRat]
        simp only [one_mul]
        rw [← zpow_add₀ (by norm_num)]
        apply (congrArg (fun x : ℤ => (2 : ℚ) ^ x))
        linarith
      · -- Case 3: Both Mantissa and Exponent overflow (m + 1 = maxNormal & e = c.emax)
        injection hs with hy; subst hy
        rw [toRat_finite_eq _ _ _ h]
        dsimp [toRat]
        simp only [Option.some_lt_some]
        apply lt_of_lt_of_le (toRat_of_finite_lt_pow h)
        apply zpow_le_zpow_right₀ (by norm_num)
        simp only [add_le_add_iff_right]
        exact ValidFinite_e_le_emax h

end Ryu.IEEE754.Float
