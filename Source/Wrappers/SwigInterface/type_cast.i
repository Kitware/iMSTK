%inline %{
/* template <typename Base, typename Derived> */
/* Derived* type_cast(Base* base) { */
/*     return dynamic_cast<Derived*>(base); */
/* } */
template <typename Base, typename Derived>
std::shared_ptr<Derived> type_cast(std::shared_ptr<Base> base) {
    return std::dynamic_pointer_cast<Derived>(base);
}
%}

%pragma(csharp) modulecode=%{
  public static T CastTo<T>(object from, bool cMemoryOwn=false)
  {
      System.Reflection.MethodInfo CPtrGetter = from.GetType().GetMethod("getCPtr", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Static);
      return CPtrGetter == null ? default(T) : (T) System.Activator.CreateInstance
      (
          typeof(T),
          System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance,
          null,
          new object[] { ((System.Runtime.InteropServices.HandleRef) CPtrGetter.Invoke(null, new object[] { from })).Handle, cMemoryOwn },
          null
      );
  }
%}
