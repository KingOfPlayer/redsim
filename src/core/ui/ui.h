
class RootUI;

class UI{
RootUI* rootUI;
public:
    virtual void render() = 0;
    UI(RootUI* rootUI) {
        this->rootUI = rootUI;
    }
};