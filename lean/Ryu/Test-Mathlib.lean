import Mathlib

open FP

-- Float16
local instance : FP.FloatCfg where
  prec := 11
  emax := 15
  precPos := by decide
  precMax := by decide

#eval (FP.prec : Nat)
#eval (FP.emax : Nat)
#eval FP.emin

variable [FloatCfg]

#check (FP.Float.nan)
#check (FP.Float.inf false)
#check (FP.Float.inf true)
#check (FP.Float.isFinite : FP.Float → Bool)

unsafe def succ (x : FP.Float) : FP.Float :=
  FP.nextUp x

#check succ

/-- Expose the internal `(sign, mantissa, exponent)` if `f` is finite. -/
def sme? (f : FP.Float) : Option (Bool × ℕ × ℤ) :=
  match f with
  | FP.Float.finite s e m _ => some (s, m, e)
  | _ => none

def toRat? (f : FP.Float) : Option ℚ :=
  match f with
  | FP.Float.finite s e m v =>
      some (FP.toRat (FP.Float.finite s e m v) (by simp [FP.Float.isFinite]))
  | _ => none

--unsafe def f0 : FP.Float := FP.ofRat FP.RMode.NE 0
unsafe def f1 := FP.ofRat FP.RMode.NE 2048
--unsafe def f2 : FP.Float := FP.ofRat FP.RMode.NE (3/2)

--#eval toRat? (FP.nextUp f1)

#eval sme? f1
#eval toRat? (FP.nextUp f1)

--#eval toRat? (FP.nextUp (FP.nextUp (FP.nextUp f1)))
--#eval sme? (FP.nextUp (FP.nextUp (FP.nextUp f1)))
