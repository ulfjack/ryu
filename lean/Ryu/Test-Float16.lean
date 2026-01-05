import Mathlib
import Ryu.IEEE754.Float

open Ryu.IEEE754.Float

def cfg16 : Cfg where
  prec := 11
  ewidth := 5
  prec_at_least_2 := by decide
  ewidth_at_least_2 := by decide

def isValid (c : Cfg) (e : Int) (m : Nat) : Bool :=
  decide (ValidFinite c e m)

local notation "⟪" neg "," e "," m "⟫" =>
  F.finite (c := cfg16) neg e m (by decide)

#eval isValid cfg16 (-14) 0      -- Zero
#eval isValid cfg16 (-14) 1      -- min subnormal
#eval isValid cfg16 (-14) 1023   -- max subnormal
#eval isValid cfg16 (-14) 1024   -- min normal
#eval isValid cfg16 (-14) 2047   -- some normal
#eval isValid cfg16 (-13) 1024   -- some normal
#eval isValid cfg16 (15) 2047    -- max normal

#eval toRat ⟪false, -14, 0⟫
#eval toRat ⟪false, -14, 1⟫
#eval toRat ⟪false, -14, 1023⟫
#eval toRat ⟪false, -14, 1024⟫
#eval toRat ⟪false, 0, 1025⟫
#eval toRat ⟪false, 15, 2047⟫
#eval toRat ⟪true, -14, 0⟫

def f0 : F cfg16 := F.finite false (-14) 1023 (by decide)
def f1 : F cfg16 := F.finite false (-14) 1024 (by decide)

#eval toRat f0
#eval toRat (succ f0)
#eval toRat f1
