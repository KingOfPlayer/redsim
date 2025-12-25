
class RootUI;

class UI{
RootUICtx* rootUI;
public:
    virtual void render() = 0;
    UI(RootUICtx* rootUICtx) {
        this->rootUI = rootUICtx;
    }
    RootUICtx* GetRootUIContext(){
        return rootUI;
    };
};