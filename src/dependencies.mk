
common=
codegen=
dot=
parser=
state=
test=codegen state parser dot common
viewer=codegen state parser dot common
matcher_builder=codegen state parser dot common


define make_depen
$(eval $1: $($1))
endef
map = $(foreach a,$(2),$(call $(1),$(a)))
define make_prereqs
$(call map,make_depen,test viewer matcher_builder codegen state parser dot common)
endef
