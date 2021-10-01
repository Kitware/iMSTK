namespace std {
template<class Ty> class weak_ptr {
public:
    typedef Ty element_type;

    weak_ptr();
    weak_ptr(const weak_ptr&);
    template<class Other>
        weak_ptr(const weak_ptr<Other>&);
    template<class Other>
        weak_ptr(const shared_ptr<Other>&);

    weak_ptr(const shared_ptr<Ty>&);


    void swap(weak_ptr&);
    void reset();

    long use_count() const;
    bool expired() const;
    shared_ptr<Ty> lock() const;
};
} /* end of namespace std */


%template(SceneManagerWeakPtr) std::weak_ptr<imstk::SceneManager>;
%template(ModuleDriverWeakPtr) std::weak_ptr<imstk::ModuleDriver>;
%template(SimulationManagerWeakPtr) std::weak_ptr<imstk::SimulationManager>;
