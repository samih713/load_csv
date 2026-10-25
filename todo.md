# TODO

- [ ] **Separate errors from the row count.** `cxv_read` returns `-1` on error
  and the number of rows read otherwise. Make it return only the row count
  and report errors another way.
- [ ] **Infer the column count.** Work out the number of columns from the file
  instead of requiring `cfg.n_cols`.
- [ ] **Remove the row length limit.** Read rows of any length instead of
  capping them at `CXV_MAX_ROW_SIZE`.
- [ ] **Add config defaults.** Unset fields should fall back to sensible
  values, e.g. `r_to_read = 0` reads the whole file.
- [ ] **Support quoted fields.** Handle delimiters inside quotes (`"a,b"`) and
  escaped quotes (`""`), per RFC 4180.
