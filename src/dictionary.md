Sorndmc
==============
- `find_cutoffs()`:			Find outlier walkers in energy distribution
- `myprop_f`, `prop`:			Fermionic properties
- `myprop_b`:				Bosonic properties
- `myprop_a`:				Absolute properties

Overall structure of `Sorndmc_method::runWithVariables`:
```
get_signs_from_PsiT();
loop over blocks {
	loop over steps {
		loop over walkers {
			loop over steps_without_branching {
				loop over electrons {
					move_using_gwf();
				}
				calculate_energies();
				update_wi_weights();	// walker_sign * Psi_T_sign ???
				update_T_G_ratio();
			}
		}
		update_Etrial();
	}	
	write_weighted_energies();
	update_Etrial();
}
```

