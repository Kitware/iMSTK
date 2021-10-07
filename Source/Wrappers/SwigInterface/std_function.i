%{
    #include <functional>
%}

// These are the things we actually use
#define param(num, type) $typemap(cstype, type) arg##num
#define unpack(num, type) arg##num
#define lvalref(num, type) type&& arg##num
#define rvalref(num, mytype) std::add_rvalue_reference<mytype>::type arg##num
#define forward(num, type) std::forward<type>(arg##num)

// This is the mechanics
#define FE_0(...)
#define FE_1(action, a1) action(0, a1)
#define FE_2(action, a1, a2) action(0, a1), action(1, a2)
#define FE_3(action, a1, a2, a3) action(0, a1), action(1, a2), action(2, a3)
#define FE_4(action, a1, a2, a3, a4) action(0, a1), action(1, a2), action(2, a3), action(3, a4)
#define FE_5(action, a1, a2, a3, a4, a5) action(0, a1), action(1, a2), action(2, a3), action(3, a4), action(4, a5)

#define GET_MACRO(_1, _2, _3, _4, _5, NAME, ...) NAME
%define FOR_EACH(action, ...) 
    GET_MACRO(__VA_ARGS__, FE_5, FE_4, FE_3, FE_2, FE_1, FE_0)(action, __VA_ARGS__)
%enddef

%define %std_function(Name, Ret, ...)
    // Define a director for Name. When an object of Name class is constructed it will
    // register its virtual function via reverse pinvoke (this passes back function pointers
    // to its functions that should be called)
    %feature("director") Name;
    %typemap(csclassmodifiers) Name "public abstract class";

    // We then create a class of Name to be wrapped with virtual function call
    // which will be reverse pinvoked called in the C# version of the class
    %{
        struct Name
        {
            virtual ~Name() {}
            virtual Ret call(__VA_ARGS__) = 0;
        };
    %}

    // Make the call pure abstract
    %csmethodmodifiers Name::call "public abstract";
    %typemap(csout) Ret Name::call ";" // Suppress the body of the abstract method

    // Provide the implementation
    // Note: The body of call in C# cannot be called (though is possible to generate it)
    struct Name
    {
        virtual ~Name();
    protected:
        virtual Ret call(__VA_ARGS__) = 0;
    };

    // We then need to wrap the std::function<...> such that it's callback
    // will actually call the virtual call function in the class Name
    %rename(Name##Std) std::function<Ret(__VA_ARGS__)>;
    %rename(call) std::function<Ret(__VA_ARGS__)>::operator();

    // Wrap the std::function class
    namespace std
    {
        struct function<Ret(__VA_ARGS__)>
        {
            // Copy constructor
            function<Ret(__VA_ARGS__)>(const std::function<Ret(__VA_ARGS__)>&);

            // Call operator
            virtual Ret operator()(__VA_ARGS__) const;

            // Conversion constructor from function pointer
            function<Ret(__VA_ARGS__)>(Ret(*const)(__VA_ARGS__));

            // The function for the std::function actually just forwards
            // to in->call here
            %extend
            {
                function<Ret(__VA_ARGS__)>(Name* in)
                {
                    return new std::function<Ret(__VA_ARGS__)>(
                        [=](FOR_EACH(lvalref, __VA_ARGS__))
                        {
                            return in->call(FOR_EACH(forward, __VA_ARGS__));
                        });
                }
            }
        };
    }
%enddef

%std_function(EventFunc, void, imstk::Event*)
%std_function(KeyEventFunc, void, imstk::KeyEvent*)