#include <memory>
template <typename U>
class WeakPtr;
struct BaseControlBlock{
    size_t shared_count = 0;
    size_t weak_count = 0;
    BaseControlBlock(size_t shared_count,size_t weak_count):shared_count(shared_count),weak_count(weak_count) { }
    BaseControlBlock() = default;
    virtual void destroy()  = 0;
    virtual void dealloc()  = 0;
    virtual ~BaseControlBlock() = default;
};

template<typename U,typename Deleter, typename Alloc>
struct ControlBlockRegular:BaseControlBlock{
    U* ptr = nullptr;
    [[no_unique_address]] Deleter del;
    [[no_unique_address]] Alloc alloc;
    ControlBlockRegular(U* ptr,Deleter del = Deleter(), Alloc alloc = Alloc()): BaseControlBlock(1, 0),ptr(ptr), del(std::move(del)), alloc(std::move(alloc)){}
    void destroy() override{
        del(ptr);
        ptr = nullptr;
    }
    void dealloc() override{
        using cbrAlloc = std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockRegular<U,Deleter,Alloc>>;
        cbrAlloc cbr(alloc);
        std::allocator_traits<cbrAlloc>::deallocate(cbr,this,1);
    }


};
template<typename U,typename Alloc>
struct ControlBlockMakeShared:BaseControlBlock{
    [[no_unique_address]] Alloc alloc;
    U value;
    template<typename... Args>
    ControlBlockMakeShared(Alloc alloc,Args&&... args):BaseControlBlock(1,0),alloc(std::move(alloc)),value(std::forward<Args>(args)...){}
    void destroy()  override{
        using elementalloc = typename std::allocator_traits<Alloc>::template rebind_alloc<U>;
        elementalloc element(alloc);
        std::allocator_traits<elementalloc>::destroy(element,&value);
    }
    void dealloc()  override{
        using cbmAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockMakeShared<U, Alloc>>;
        cbmAlloc cbm(alloc);
        std::allocator_traits<cbmAlloc>::deallocate(cbm,this,1);
    }


};
template <typename T>
class SharedPtr{
private:
    template<typename U>
    friend class WeakPtr;
    template<typename U>
    friend class SharedPtr;

    T* ptr = nullptr;
    BaseControlBlock* pcb = nullptr;


    template<typename U>
    SharedPtr(U* ptr,BaseControlBlock* cb):ptr(ptr),pcb(cb) {
        ++cb->shared_count;
    }

    template<typename U,typename Alloc>
    SharedPtr(ControlBlockMakeShared<U,Alloc>* cb):ptr(&cb->value),pcb(cb){}
    template<typename U,typename Alloc,typename... Args>
    friend SharedPtr<U> allocateShared(const Alloc& alloc,Args&&... args);


public:

    SharedPtr() = default;
    template<typename U, typename Deleter, typename Alloc>
    SharedPtr(U* ptr,Deleter del, Alloc alloc):ptr(ptr){
        using Alloc_cb = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockRegular<T, Deleter, Alloc>>;
        Alloc_cb allocCb(alloc);
        auto create_pcb = std::allocator_traits<Alloc_cb>::allocate(allocCb, 1);
        /*std::allocator_traits<Alloc_cb>::construct(allocCb, create_pcb,
                                                   ControlBlockRegular<T, Deleter, Alloc>(ptr, std::move(del), std::move(alloc)));*/
        new (create_pcb) ControlBlockRegular<T, Deleter, Alloc>(ptr, std::move(del), std::move(alloc));
        pcb = create_pcb;
    }

    template<typename U,typename Deleter>
    SharedPtr(U* ptr,Deleter del):SharedPtr(ptr,std::move(del),std::allocator<U>()){}

    template<typename U>
    SharedPtr(U* ptr):SharedPtr(ptr,std::default_delete<U>(),std::allocator<U>()){}

    template<typename U>
    SharedPtr(const SharedPtr<U>& other) noexcept:ptr(other.ptr),pcb(other.pcb){
        if (pcb) {
            ++pcb->shared_count;
        }
    }

    SharedPtr(const SharedPtr& other) noexcept:ptr(other.ptr),pcb(other.pcb){
        if (pcb) {
            ++pcb->shared_count;
        }
    }

    SharedPtr(SharedPtr&& other) noexcept:ptr(other.ptr),pcb(other.pcb){
        other.ptr = nullptr;
        other.pcb = nullptr;
    }
    template<typename U>
    SharedPtr(SharedPtr<U>&& other) noexcept:ptr(other.ptr),pcb(other.pcb){
        other.ptr = nullptr;
        other.pcb = nullptr;
    }
    SharedPtr& operator=(const SharedPtr& other) noexcept{
        auto copy(other);
        swap(copy);
        return *this;
    }
    SharedPtr& operator=(SharedPtr&& other) noexcept {
        auto copy(std::move(other));
        swap(copy);
        return *this;
    }
    template<typename U>
    SharedPtr& operator=(const SharedPtr<U>& other) noexcept{
        SharedPtr copy(other);
        swap(copy);
        return *this;
    }
    template<typename U>
    SharedPtr& operator=(SharedPtr<U>&& other) noexcept {
        SharedPtr copy(std::move(other));
        swap(copy);
        return *this;
    }
    T* get() const{
        return ptr;
    }
    template<typename U>
    void swap(SharedPtr<U>& other){
        std::swap(ptr,other.ptr);
        std::swap(pcb,other.pcb);
    }
    T* operator->() const{
        return ptr;
    }
    T& operator*() const{
        return *ptr;
    }
    size_t use_count() const{
        return pcb->shared_count;
    }

    template<typename U>
    void reset(U* ptr_){
        *this = ptr_;
    }
    void reset(){
        *this = SharedPtr();
    }




    ~SharedPtr(){
        if (!pcb||pcb->shared_count==0){
            return;
        }
        --pcb->shared_count;
        if (pcb->shared_count==0){
            pcb->destroy();
            if (pcb->weak_count==0){
                pcb->dealloc();
            }
        }
    }
};

template<typename T,typename Alloc,typename... Args>
SharedPtr<T> allocateShared(const Alloc& alloc,Args&&... args){

    using allocBlock = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockMakeShared<T,Alloc>>;
    allocBlock alloc_block(alloc);

    auto* p = std::allocator_traits<allocBlock>::allocate(alloc_block,1);
    std::allocator_traits<allocBlock>::construct(alloc_block,p,alloc,std::forward<Args>(args)...);
    return p;
}

template<typename T,typename... Args>
SharedPtr<T> makeShared(Args&&... args){
    return allocateShared<T>(std::move(std::allocator<T>()),std::forward<Args>(args)...);
}



template <typename T>
class WeakPtr {
    T* ptr = nullptr;
    BaseControlBlock* pcb = nullptr;
    template<typename U>
    friend class WeakPtr;
public:
    WeakPtr() = default;

    WeakPtr(const WeakPtr& other):ptr(reinterpret_cast<T*>(other.ptr)),pcb(other.pcb){
        ++pcb->weak_count;
    }

    template<typename U>
    WeakPtr(const SharedPtr<U>& sp):ptr(reinterpret_cast<T*>(sp.ptr)),pcb(sp.pcb){
        ++pcb->weak_count;
    }

    template<typename U>
    WeakPtr(const WeakPtr<U>& other):ptr(reinterpret_cast<T*>(other.ptr)),pcb(other.pcb){
        ++pcb->weak_count;
    }
    WeakPtr& operator=(const WeakPtr& other){
        this->~WeakPtr();
        ptr = other.ptr;
        pcb = other.pcb;
        ++pcb->weak_count;
        return *this;
    }

    template<typename U>
    WeakPtr& operator=(const SharedPtr<U>& other){
        this->~WeakPtr();
        ptr = reinterpret_cast<T*>(other.ptr);
        pcb = other.pcb;
        ++pcb->weak_count;
        return *this;
    }

    bool expired() const{
        return use_count()==0;
    }
    SharedPtr<T> lock() const{
        return SharedPtr<T>(ptr,pcb);
    }
    size_t use_count() const{
        return pcb->shared_count;
    }
    ~WeakPtr(){
        if (pcb){
            --pcb->weak_count;
            if(pcb->shared_count==0&&pcb->weak_count==0){
                pcb->dealloc();
            }
        }
    }
};