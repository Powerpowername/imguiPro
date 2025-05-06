#include"All.h"

// ====================== Input 输入管理类 ======================
#pragma region Input

// 静态成员变量初始化（键盘输入状态数组）
bool* Input::keyDown = new bool[KEYS];   // 按键按下（仅当前帧触发）
bool* Input::Key = new bool[KEYS];       // 当前帧按键状态
bool* Input::lastKey = new bool[KEYS];   // 上一帧按键状态
bool* Input::keyUp = new bool[KEYS];     // 按键释放（仅当前帧触发）
glm::vec2 Input::mouseMentDelta = glm::vec2(0, 0); // 鼠标移动增量

// 获取按键当前状态（是否按下）
bool Input::GetKey(int key) {
    return Key[key];
}

// 获取按键释放状态（当前帧刚释放）
bool Input::GetKeyUp(int key) {
    return keyUp[key];
}

// 获取按键按下状态（当前帧刚按下）
bool Input::GetKeyDown(int key) {
    return keyDown[key];
}

// 更新输入状态（比较当前帧与上一帧状态）
void Input::GetInput() {
    for (int i = 0; i < KEYS; i++) {
        // 检测按键按下事件（当前帧按下，上一帧未按下）
        if (Key[i] && !lastKey[i])
            keyDown[i] = true;
        // 检测按键释放事件（当前帧未按下，上一帧按下）
        if (!Key[i] && lastKey[i])
            keyUp[i] = true;
    }
}

// 每帧清除临时输入状态（保留当前按键状态到lastKey）
void Input::ClearInputEveryFrame() {
    // 保存当前状态到lastKey
    memcpy(lastKey, Key, sizeof(bool)*KEYS);
    // 重置当前状态和临时状态
    memset(Key, 0, sizeof(bool)*KEYS);
    memset(keyDown, 0, sizeof(bool)*KEYS);
    memset(keyUp, 0, sizeof(bool)*KEYS);
    mouseMentDelta = vec2(0, 0); // 重置鼠标移动增量
}

// 初始化输入系统（重置所有状态）
void Input::InitInput() {
    memset(Key, 0, sizeof(bool)*KEYS);
    memset(keyDown, 0, sizeof(bool)*KEYS);
    memset(keyUp, 0, sizeof(bool)*KEYS);
    memset(lastKey, 0, sizeof(bool)*KEYS);
}

#pragma endregion


// ====================== MonoBehavior 行为脚本基类 ======================
#pragma region Monobehavior ： Object

// 构造函数：初始化名称和所属游戏对象
MonoBehavior::MonoBehavior() : Object("MonoBehavior_") {
    this->gameObject = nullptr; // 初始时无所属游戏对象
}

// 析构函数（虚析构，确保派生类资源释放）
MonoBehavior::~MonoBehavior() {}

// 初始化逻辑（虚函数，派生类可重写）
void MonoBehavior::Start() {}

// 物理更新（虚函数，用于固定时间步逻辑）
void MonoBehavior::RealUpdate() {}

// 每帧更新（虚函数，随帧率调用）
void MonoBehavior::Update() {}

// ImGui 调试界面（虚函数，显示启用状态复选框）
void MonoBehavior::OnGUI() const {
    ImGui::Checkbox("Enable", (bool*)&enable); // 显示启用状态切换复选框
}

#pragma endregion


// ====================== Transform 变换组件 ======================
#pragma region Transform : MonoBehavior

// 平移操作（世界空间平移）
void Transform::Translate(vec3 movement) {
    position += movement; // 直接更新位置
}

// 获取模型矩阵（包含平移、旋转、缩放，支持世界矩阵叠加）
mat4 Transform::GetModelMaterix(mat4 world) const {
    mat4 model = world;
    model = translate(model, position);       // 平移
    model = glm::scale(model, scale);         // 缩放
    // 按顺序应用欧拉角旋转（X-Y-Z轴）
    model = rotate(model, radians(rotation.x), vec3(1, 0, 0)); // X轴旋转
    model = rotate(model, radians(rotation.y), vec3(0, 1, 0)); // Y轴旋转
    model = rotate(model, radians(rotation.z), vec3(0, 0, 1)); // Z轴旋转
    return model;
}

// ImGui 调试界面（显示变换参数）
void Transform::OnGUI() const {
    MonoBehavior::OnGUI(); // 显示基类的启用状态复选框
    // 拖动条显示位置、旋转、缩放参数
    ImGui::DragFloat3((gameObject->name + "_position").c_str(), (float*)&position, 0.5f, -500, 500);
    ImGui::DragFloat3((gameObject->name + "_rotation").c_str(), (float*)&rotation, 0.5f, -500, 500);
    ImGui::DragFloat3((gameObject->name + "_scale").c_str(), (float*)&scale, 0.01f, -100, 100);
    // 显示相机相关的偏航角和俯仰角
    ImGui::DragFloat((gameObject->name + "_Yaw").c_str(), (float*)&Yaw, 0.01f, -10, 10);
    ImGui::DragFloat((gameObject->name + "_Pitch").c_str(), (float*)&Pitch, 0.01f, -10, 10);
}

// 物理更新（计算世界空间方向向量）
void Transform::RealUpdate() {
    MonoBehavior::RealUpdate();
    // 根据欧拉角计算前向向量
    Forward.x = cos(Pitch) * sin(Yaw);
    Forward.y = sin(Pitch);
    Forward.z = cos(Pitch) * cos(Yaw);
    // 计算右向和上向向量（基于世界上方向）
    Right = normalize(cross(Forward, WorldUp)); // 右向 = 前向 × 世界上向
    Up = normalize(cross(Forward, Right));      // 上向 = 前向 × 右向（确保正交）
}

// 构造函数（初始化变换参数并计算初始方向）
Transform::Transform(vec3 pos, vec3 rotation, vec3 scale) 
    : position(pos), rotation(rotation), scale(scale) {
    name += "Transform"; // 设置组件名称
    // 初始方向计算（与RealUpdate逻辑一致）
    Forward.x = cos(Pitch) * sin(Yaw);
    Forward.y = sin(Pitch);
    Forward.z = cos(Pitch) * cos(Yaw);
    Right = normalize(cross(Forward, WorldUp));
    Up = normalize(cross(Right, Forward));
}

// 析构函数（无资源释放，留空）
Transform::~Transform() {}

#pragma endregion


// ====================== Camera 相机类 ======================
#pragma region Camera : MonoBehavior

// 构造函数（设置默认名称）
Camera::Camera() {
    name += "Camera"; // 设置组件名称
}

// 析构函数（无资源释放，留空）
Camera::~Camera() {}

// 初始化（设置初始相机参数）
void Camera::Start() {
    MonoBehavior::Start();
    // 获取所属游戏对象的Transform组件
    transform = gameObject->transform();
    // 设置初始视角（仰角15度，偏航角180度，看向-Z方向）
    transform->Pitch = radians(15.0f);
    transform->Yaw = radians(180.0f);
    // 设置视口为窗口大小
    viewPort = vec4(0, 0, pWindowSize->x, pWindowSize->y);
}

// 物理更新（更新视角和投影矩阵）
void Camera::RealUpdate() {
    MonoBehavior::RealUpdate();
    // 设置OpenGL视口
    glViewport(viewPort.x, viewPort.y, viewPort.z, viewPort.w);
    // 计算视图矩阵（从相机视角看世界）
    viewMat = lookAt(transform->position, transform->position + transform->Forward, transform->WorldUp);
    // 计算透视投影矩阵（视角、宽高比、近远裁剪平面）
    projMat = perspective(radians(this->angle), viewPort.z / viewPort.w, near, far);
}

// ImGui 调试界面（显示相机参数）
void Camera::OnGUI() const {
    MonoBehavior::OnGUI(); // 显示基类的启用状态复选框
    // 拖动条显示视口、视角、近远裁剪平面
    ImGui::DragFloat4(("viewPort" + std::to_string(gameObject->id)).c_str(), (float*)&viewPort, 10, 0, 2000);
    ImGui::DragFloat(("viewAngle" + std::to_string(gameObject->id)).c_str(), (float*)&angle, 3, 0, 180.0f);
    ImGui::DragFloat(("near" + std::to_string(gameObject->id)).c_str(), (float*)&near, 0.01f, 0, 10);
    ImGui::DragFloat(("far" + std::to_string(gameObject->id)).c_str(), (float*)&far, 1.0f, 0, 1000);
}

#pragma endregion


// ====================== CameraMove 相机移动脚本 ======================
#pragma region CameraMove : MonoBehvaior

// 初始化（获取所属游戏对象的Transform组件）
void CameraMove::Start() {
    MonoBehavior::Start();
    transform = gameObject->transform(); // 获取相机的变换组件
}

// 每帧更新（处理键盘输入控制相机移动）
void CameraMove::Update() {
    MonoBehavior::Update();
    // 根据Shift键切换移动速度
    if (Input::GetKeyDown(Shift_))
        currentSpeed = highSpeed; // 高速移动
    if (Input::GetKeyUp(Shift_))
        currentSpeed = normalSpeed; // 恢复正常速度

    // 键盘WASDQE控制相机移动（基于Transform的方向向量）
    if (Input::GetKey(S_))  transform->Translate(-currentSpeed * transform->Forward * Setting::deltaTime); // 后移
    if (Input::GetKey(W_))  transform->Translate(currentSpeed * transform->Forward * Setting::deltaTime);  // 前移
    if (Input::GetKey(A_))  transform->Translate(-currentSpeed * transform->Right * Setting::deltaTime);   // 左移
    if (Input::GetKey(D_))  transform->Translate(currentSpeed * transform->Right * Setting::deltaTime);    // 右移
    if (Input::GetKey(Q_))  transform->Translate(-currentSpeed * transform->Up * Setting::deltaTime);      // 下移
    if (Input::GetKey(E_))  transform->Translate(currentSpeed * transform->Up * Setting::deltaTime);       // 上移

    // 空格键切换鼠标锁定状态（控制鼠标是否隐藏）
    if (Input::GetKeyDown(Space_)) {
        Setting::lockMouse = !Setting::lockMouse;
        // 设置GLFW鼠标模式（禁用或正常）
        glfwSetInputMode(window, GLFW_CURSOR, Setting::lockMouse ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}

// 物理更新（处理鼠标移动控制相机旋转）
void CameraMove::RealUpdate() {
    MonoBehavior::RealUpdate();
    if (!Setting::lockMouse) return; // 未锁定鼠标时跳过
    // 根据鼠标移动增量更新欧拉角（反转Y轴，应用灵敏度）
    transform->Pitch -= Input::mouseMentDelta.y * sencity;
    transform->Yaw -= Input::mouseMentDelta.x * sencity;
}

// ImGui 调试界面（显示移动控制参数）
void CameraMove::OnGUI() const {
    MonoBehavior::OnGUI(); // 显示基类的启用状态复选框
    // 拖动条显示灵敏度和移动速度
    ImGui::DragFloat("sencity", (float*)&sencity, 0.001f, 0, 0.010f);
    ImGui::DragFloat("normalSpeed", (float*)&normalSpeed, 1, 1, 20);
    ImGui::DragFloat("highSpeed", (float*)&highSpeed, 1, 5, 50);
}

// 构造函数（设置组件名称）
CameraMove::CameraMove() {
    name += "CameraMove"; // 设置组件名称
}

// 析构函数（无资源释放，留空）
CameraMove::~CameraMove() {}

#pragma endregion


// ====================== Rotate 旋转脚本 ======================
#pragma region Rotate : MonoBehavior

// ImGui 调试界面（显示旋转速度）
void Rotate::OnGUI() const {
    MonoBehavior::OnGUI(); // 显示基类的启用状态复选框
    ImGui::DragFloat("rotateSpeed", (float*)&rotateSpeed, 0.05f); // 拖动条调整旋转速度
}

// 每帧更新（绕Y轴旋转游戏对象）
void Rotate::Update() {
    MonoBehavior::Update();
    // 直接修改Transform的旋转角度（绕Y轴）
    gameObject->transform()->rotation += vec3(0, rotateSpeed, 0);
}

// 构造函数（设置组件名称）
Rotate::Rotate() {
    name += "Rotate"; // 设置组件名称
}

// 析构函数（无资源释放，留空）
Rotate::~Rotate() {}

#pragma endregion


// ====================== AbstractLight 抽象光照类 ======================
#pragma region AbstractLight ：MonoBehavior

// 更新光照方向（根据Transform的旋转计算）
void AbstractLight::UpdateDirection() {
    // 初始方向为Z轴正方向，经旋转后取反（指向光源照射方向）
    direction = vec3(0, 0, 1.0f);
    direction = rotateZ(direction, transform->rotation.z); // Z轴旋转
    direction = rotateX(direction, transform->rotation.x); // X轴旋转
    direction = rotateY(direction, transform->rotation.y); // Y轴旋转
    direction *= -1; // 方向取反（指向光源照射方向）
}

// 序列化到JSON（存储基本光照参数）
void AbstractLight::ToJson(json & j) const {
    j = json{
        {"direction", direction},  // 光照方向
        {"color", color},          // 光照颜色
        {"strength", strength}     // 光照强度
    };
}

// 从JSON反序列化（读取基本光照参数）
void AbstractLight::FromJson(const json & j) {
    color = j.at("color").get<vec3>();       // 读取颜色
    direction = j.at("direction").get<vec3>(); // 读取方向
    strength = j.at("strength").get<float>(); // 读取强度
}

// 构造函数（将自身添加到全局光照列表）
AbstractLight::AbstractLight() : color(vec3(1, 1, 1)) {
    Setting::lights->push_back(this); // 添加到全局光照列表
}

// 析构函数（调用基类析构）
AbstractLight::~AbstractLight() {
    MonoBehavior::~MonoBehavior();
}

// ImGui 调试界面（显示光照基本参数）
void AbstractLight::OnGUI() const {
    MonoBehavior::OnGUI(); // 显示基类的启用状态复选框
    ImGui::SliderFloat("DirStrength", (float*)&strength, 0.0f, 10.0f); // 强度滑块
    ImGui::DragFloat3("DirAngles", (float*)&transform->rotation, 0.1f, 0, 2 * 3.1415926535f); // 方向角度拖动条
    ImGui::ColorEdit3("LightColor", (float*)&color); // 颜色选择器
}

// 初始化（获取所属游戏对象的Transform组件）
void AbstractLight::Start() {
    MonoBehavior::Start();
    transform = gameObject->transform(); // 获取变换组件
}

// 每帧更新（调用UpdateDirection计算方向）
void AbstractLight::Update() {
    MonoBehavior::Update();
    UpdateDirection(); // 更新光照方向
}

// 设置着色器参数（派生类需重写，此处提供基础实现）
void AbstractLight::SetShader(Shader * shader, int index) {
    // 启用光照标志，传递颜色、位置、方向
    shader->setBool(Sign() + "[" + std::to_string(index) + "]." + "flag", true);
    shader->setVec3(Sign() + "[" + std::to_string(index) + "]." + "color", this->color * strength);
    shader->setVec3(Sign() + "[" + std::to_string(index) + "]." + "pos", transform->position);
    shader->setVec3(Sign() + "[" + std::to_string(index) + "]." + "dirToLight", this->direction);
}

// JSON序列化友元函数（允许直接读写AbstractLight对象）
void to_json(json & j, const AbstractLight & l) {
    l.ToJson(j); // 调用对象的序列化方法
}

void from_json(const json & j, AbstractLight & l) {
    l.FromJson(j); // 调用对象的反序列化方法
}

#pragma endregion


// ====================== DirectionalLight 方向光 ======================
#pragma region DirectionalLight : AbstractLight

// 构造函数（设置组件名称）
LightDirectional::LightDirectional() {
    name += "DirectionalLight"; // 设置组件名称
}

// 析构函数（无资源释放，留空）
LightDirectional::~LightDirectional() {}

// 初始化（设置默认位置和旋转）
void LightDirectional::Start() {
    AbstractLight::Start();
    // 设置初始位置和旋转（形成45度角照射）
    transform->position = vec3(1, 1, -1);
    transform->rotation = vec3(radians(45.0f), radians(90.0f), radians(0.0f));
    color = vec3(1, 1, 1); // 白色光照
}

// JSON序列化友元函数（实现具体序列化逻辑，此处与基类一致）
void to_json(json & j, const LightDirectional & l) {
    j = json{
        {"direction", l.direction},
        {"color", l.color},
        {"strength", l.strength}
    };
}

void from_json(const json & j, LightDirectional & l) {
    l.color = j.at("color").get<vec3>();
    l.direction = j.at("direction").get<vec3>();
    l.strength = j.at("strength").get<float>();
}

#pragma endregion


// ====================== PointLight 点光源 ======================
#pragma region PointLight : AbstractLight

// 构造函数（调用基类构造，设置组件名称）
LightPoint::LightPoint() : AbstractLight() {
    name += "PointLight"; // 设置组件名称
}

// 析构函数（无资源释放，留空）
LightPoint::~LightPoint() {}

// 设置着色器参数（添加衰减参数）
void LightPoint::SetShader(Shader * shader, int index) {
    AbstractLight::SetShader(shader, index); // 调用基类实现
    // 传递衰减参数到着色器
    shader->setFloat(Sign() + "[" + std::to_string(index) + "]." + "constant", this->constant);
    shader->setFloat(Sign() + "[" + std::to_string(index) + "]." + "linear", this->linear);
    shader->setFloat(Sign() + "[" + std::to_string(index) + "]." + "quadratic", this->quadratic);
}

// 初始化（设置默认位置和颜色）
void LightPoint::Start() {
    AbstractLight::Start();
    transform->position = vec3(0, 0, 10); // 初始位置在Z轴正方向10单位
    color = vec3(5, 5, 5); // 高强度白色光照
}

// ImGui 调试界面（显示衰减参数）
void LightPoint::OnGUI() const {
    AbstractLight::OnGUI(); // 显示基类的光照参数
    // 拖动条显示衰减系数
    ImGui::DragFloat("constant", (float*)&constant, 0.01f, 0, 2);
    ImGui::DragFloat("linear", (float*)&linear, 0.1f, 0, 10);
    ImGui::DragFloat("quadratic", (float*)&quadratic, 0.01f, 0, 0.5f);
}

// 从JSON反序列化（读取衰减参数）
void LightPoint::FromJson(const json & j) {
    AbstractLight::FromJson(j); // 读取基类参数
    constant = j.at("constant").get<float>(); // 读取常数衰减
    linear = j.at("linear").get<float>();     // 读取线性衰减
    quadratic = j.at("quadratic").get<float>();// 读取二次衰减
}

// 序列化到JSON（添加衰减参数）
void LightPoint::ToJson(json & j) const {
    AbstractLight::ToJson(j); // 写入基类参数
    j.push_back({ "constant", constant });
    j.push_back({ "linear", linear });
    j.push_back({ "quadratic", quadratic });
}

#pragma endregion


// ====================== SpotLight 聚光灯 ======================
#pragma region SpotLight : PointLight

// 初始化（设置默认位置和旋转）
void LightSpot::Start() {
    LightPoint::Start(); // 调用父类初始化
    // 设置初始位置和旋转（指向下方）
    transform->position = vec3(5, 3, 20);
    transform->rotation = vec3(radians(90.0f), radians(0.0f), radians(0.0f));
}

// ImGui 调试界面（显示角度参数）
void LightSpot::OnGUI() const {
    LightPoint::OnGUI(); // 显示父类的衰减参数
    // 拖动条显示圆锥角度的cos值
    ImGui::DragFloat("cosPhyInner", (float*)&cosPhyInner, 0.01f, 0, 1);
    ImGui::DragFloat("cosPhyOuter", (float*)&cosPhyOuter, 0.01f, 0, 1);
}

// 构造函数（设置组件名称）
LightSpot::LightSpot() {
    name = "MonoBehavior_SpotLight"; // 设置组件名称
}

// 析构函数（无资源释放，留空）
LightSpot::~LightSpot() {}

// 从JSON反序列化（读取角度参数）
void LightSpot::FromJson(const json & j) {
    LightPoint::FromJson(j); // 读取父类参数
    cosPhyInner = j.at("cosPhyInner").get<float>(); // 读取内圆锥角度cos值
    cosPhyOuter = j.at("cosPhyOuter").get<float>(); // 读取外圆锥角度cos值
}

// 序列化到JSON（添加角度参数）
void LightSpot::ToJson(json & j) const {
    LightPoint::ToJson(j); // 写入父类参数
    j.push_back({ "cosPhyInner", cosPhyInner });
    j.push_back({ "cosPhyOuter", cosPhyOuter });
}

// 设置着色器参数（添加角度参数）
void LightSpot::SetShader(Shader * shader, int index) {
    LightPoint::SetShader(shader, index); // 调用父类实现
    // 传递圆锥角度参数到着色器
    shader->setFloat(Sign() + "[" + std::to_string(index) + "]." + "cosPhyInner", this->cosPhyInner);
    shader->setFloat(Sign() + "[" + std::to_string(index) + "]." + "cosPhyOuter", this->cosPhyOuter);
}

#pragma endregion


// ====================== Mesh 网格类 ======================
#pragma region Mesh

// 绘制网格（绑定纹理和顶点数据，执行渲染）
void Mesh::Draw(Shader * shader) {
    // 绑定纹理（处理不同类型的纹理：漫反射、高光、法线、高度）
    unsigned int diffuseNr = 0, specularNr = 0, normalNr = 0, heightNr = 0;
    for (unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i); // 激活纹理单元
        string name = textures[i].type;
        string number;
        // 根据纹理类型生成编号（如diffuse_texture0）
        if (name == "texture_diffuse") number = std::to_string(diffuseNr++);
        else if (name == "texture_specular") number = std::to_string(specularNr++);
        else if (name == "texture_normal") number = std::to_string(normalNr++);
        else if (name == "texture_height") number = std::to_string(heightNr++);
        // 设置着色器采样器对应的纹理单元
        shader->setInt("material." + (name + number), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id); // 绑定纹理
    }

    // 绘制网格
    glBindVertexArray(vao); // 绑定顶点数组对象
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0); // 绘制三角形
    glBindVertexArray(0); // 解绑

    glActiveTexture(GL_TEXTURE0); // 恢复默认纹理单元
}

// 构造函数（从顶点数组初始化）
Mesh::Mesh(float vertices[]) {
    this->vertices.resize(36); // 假设顶点数固定为36（根据实际需求调整）
    memcpy(&(this->vertices[0]), vertices, 36 * 8 * sizeof(float)); // 复制顶点数据
    SetUpMesh(); // 初始化OpenGL对象
}

// 构造函数（从顶点、索引、纹理列表初始化）
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
    : vertices(vertices), indices(indices), textures(textures) {
    SetUpMesh(); // 初始化OpenGL对象
}

// 默认构造函数（留空）
Mesh::Mesh() {}

// 析构函数（无资源释放，OpenGL对象在外部管理）
Mesh::~Mesh() {}

// 初始化OpenGL对象（VAO/VBO/EBO，设置顶点属性指针）
void Mesh::SetUpMesh() {
    glGenVertexArrays(1, &vao); // 生成顶点数组对象
    glGenBuffers(1, &vbo); // 生成顶点缓冲对象
    glGenBuffers(1, &ebo); // 生成索引缓冲对象

    glBindVertexArray(vao); // 绑定VAO

    // 绑定顶点数据
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW); // 上传顶点数据

    // 绑定索引数据
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW); // 上传索引数据

    // 设置顶点属性指针（位置、法线、纹理坐标、切线、副切线）
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0); // 位置
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)); // 法线
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord)); // 纹理坐标
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent)); // 切线
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent)); // 副切线

    glBindVertexArray(0); // 解绑VAO
}

#pragma endregion


// ====================== Model 模型类 ======================
#pragma region Model : Object

// 加载模型文件（通过Assimp库解析模型）
void Model::LoadModel(string path) {
    std::cout << path << std::endl;
    Assimp::Importer importer;
    // 读取模型文件，应用后处理（翻转UV、三角化、计算切线空间）
    const aiScene* scene = importer.ReadFile(path, aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "Assimp error" << std::endl;
        return; // 加载失败
    }

    Directory = path.substr(0, path.find_last_of('\\')); // 获取模型文件目录
    name += path.substr(path.find_last_of('\\') + 1, path.length()); // 设置模型名称
    ProcessNode(scene->mRootNode, scene); // 递归处理模型节点
}

// 处理模型节点（递归遍历子节点和网格）
void Model::ProcessNode(aiNode * node, const aiScene * scene) {
    // 处理当前节点的所有网格
    for (size_t i = 0; i < node->mNumMeshes; i++) {
        aiMesh * curMesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processAiMesh(curMesh, scene)); // 转换Assimp网格到自定义Mesh
    }
    // 递归处理子节点
    for (size_t i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene);
    }
}

// 转换Assimp网格到自定义Mesh（提取顶点、索引、纹理）
Mesh Model::processAiMesh(aiMesh * aiMesh, const aiScene * aiscene) {
    std::vector<Vertex> temVertexes;
    std::vector<unsigned int> tempIndices;
    std::vector<Texture> tempTextures;

    Vertex tempVer;
    // 提取顶点数据（位置、法线、纹理坐标、切线）
    for (size_t i = 0; i < aiMesh->mNumVertices; i++) {
        tempVer.position = vec3(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);
        tempVer.normal = vec3(aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z);
        tempVer.texCoord = aiMesh->mTextureCoords[0] ? vec2(aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y) : vec2(0, 0);
        // 提取切线和副切线（如果存在）
        if (aiMesh->HasTangentsAndBitangents()) {
            tempVer.tangent = vec3(aiMesh->mTangents[i].x, aiMesh->mTangents[i].y, aiMesh->mTangents[i].z);
            tempVer.bitangent = vec3(aiMesh->mBitangents[i].x, aiMesh->mBitangents[i].y, aiMesh->mBitangents[i].z);
        }
        temVertexes.push_back(tempVer);
    }

    // 提取索引数据（三角形索引）
    for (size_t i = 0; i < aiMesh->mNumFaces; i++) {
        for (size_t j = 0; j < aiMesh->mFaces[i].mNumIndices; j++) {
            tempIndices.push_back(aiMesh->mFaces[i].mIndices[j]);
        }
    }

    // 加载材质纹理（漫反射、高光、法线、高度）
    aiMaterial* material = aiscene->mMaterials[aiMesh->mMaterialIndex];
    tempTextures.insert(tempTextures.end(), loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse").begin(), loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse").end());
    tempTextures.insert(tempTextures.end(), loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular").begin(), loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular").end());
    tempTextures.insert(tempTextures.end(), loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal").begin(), loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal").end());
    tempTextures.insert(tempTextures.end(), loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height").begin(), loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height").end());

    return Mesh(temVertexes, tempIndices, tempTextures); // 创建并返回Mesh对象
}

// 加载材质纹理（避免重复加载）
std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName) {
    std::vector<Texture> textures;
    if (mat->GetTextureCount(type) == 0) {
        // 如果没有纹理，使用默认纹理（示例逻辑，实际项目中可能需要处理错误）
        Texture texture;
        texture.id = TextureFromFile((typeName + ".jpg").c_str(), this->Directory);
        texture.type = typeName;
        texture.path = typeName.c_str();
        textures.push_back(texture);
        textures_loaded.push_back(texture);
    } else {
        // 加载所有纹理，避免重复
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);
            bool skip = false;
            // 检查纹理是否已加载
            for (unsigned int j = 0; j < textures_loaded.size(); j++) {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip) {
                // 加载新纹理
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->Directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }
    }
    return textures;
}

// ImGui 调试界面（显示模型名称）
void Model::OnGUI() const {
    if (ImGui::TreeNode(name.c_str())) { // 可展开的树节点
        ImGui::Text("name: %s", name.c_str()); // 显示模型名称
        ImGui::TreePop(); // 结束树节点
        ImGui::Spacing(); // 增加间距
    }
}

// 绘制模型（渲染所有网格）
void Model::Draw(Shader * shader) {
    for (size_t i = 0; i < meshes.size(); i++) {
        meshes[i].Draw(shader); // 绘制每个网格
    }
}

// 构造函数（加载模型文件）
Model::Model(string path) : Object("Model_") {
    LoadModel(path); // 调用加载模型方法
}

// 析构函数（无资源释放，Mesh资源在Mesh类中管理）
Model::~Model() {}

#pragma endregion


// ====================== Shader 着色器类 ======================
#pragma region Shader : Object

// 添加光照到着色器（调用光照对象的SetShader方法）
void Shader::AddLight(AbstractLight * light) {
    light->SetShader(this, Setting::LightCount(light->Type()) - 1); // 传递索引（基于类型的光照计数）
}

// ImGui 调试界面（显示着色器名称）
void Shader::OnGUI() const {
    if (ImGui::TreeNode(name.c_str())) { // 可展开的树节点
        ImGui::Text("name: %s", name.c_str()); // 显示着色器名称
        ImGui::TreePop(); // 结束树节点
        ImGui::Spacing(); // 增加间距
    }
}

// 构造函数（编译顶点、片段、几何着色器）
Shader::Shader(string sign, const char* geometryPath) : Object("Shader_" + sign) {
    std::cout << "Shader Name: " << sign << std::endl;
    std::string vertexCode, fragmentCode, geometryCode;
    // 读取着色器文件
    try {
        std::ifstream vShaderFile(sign + ".vert"), fShaderFile(sign + ".frag");
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        vShaderFile.close();
        fShaderFile.close();
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        // 加载几何着色器（如果提供路径）
        if (geometryPath != nullptr) {
            std::ifstream gShaderFile(geometryPath);
            std::stringstream gShaderStream;
            gShaderStream << gShaderFile.rdbuf();
            gShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    } catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    // 编译顶点和片段着色器
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexCode.c_str(), NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX"); // 检查编译错误

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentCode.c_str(), NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT"); // 检查编译错误

    // 编译几何着色器（如果存在）
    unsigned int geometry = 0;
    if (geometryPath != nullptr) {
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &geometryCode.c_str(), NULL);
        glCompileShader(geometry);
        checkCompileErrors(geometry, "GEOMETRY"); // 检查编译错误
    }

    // 链接着色器程序
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if (geometryPath != nullptr) glAttachShader(ID, geometry);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM"); // 检查链接错误

    // 清理临时着色器对象
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometryPath != nullptr) glDeleteShader(geometry);
}

// 激活着色器程序
void Shader::use() {
    glUseProgram(ID); // 设置当前使用的着色器程序
}

// 设置Uniform布尔值
void Shader::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); // 获取位置并设置值
}

// 其他set方法类似，均通过glUniform设置对应类型的Uniform变量（此处省略重复注释，逻辑一致）

// 检查编译/链接错误
void Shader::checkCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success); // 检查编译状态
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog); // 获取错误信息
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success); // 检查链接状态
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog); // 获取错误信息
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << std::endl;
        }
    }
}

#pragma endregion


// ====================== AbstractMaterial 抽象材质类 ======================
#pragma region AbstractMaterial : Object

// 构造函数（初始化着色器和材质参数）
AbstractMaterial::AbstractMaterial(Shader * shader, vec3 color, float skininess)
    : Object("Material_"), shader(shader), shininess(skininess), color(color) {}

// ImGui 调试界面（显示材质基本参数）
void AbstractMaterial::OnGUI() const {
    if (ImGui::TreeNode(name.c_str())) { // 可展开的树节点
        ImGui::Text("name: %s", name.c_str()); // 显示材质名称
        ImGui::Checkbox("EnableSpecular", (bool*)&specular); // 高光反射启用状态
        ImGui::DragFloat("Skininess", (float*)&shininess, 0.5f, 1, 64); // 光泽度拖动条
        ImGui::ColorEdit3((name + "_color").c_str(), (float*)&color); // 颜色选择器
        shader->OnGUI(); // 显示关联的着色器信息
        ImGui::TreePop(); // 结束树节点
        ImGui::Spacing(); // 增加间距
    }
}

// 析构函数（无资源释放，着色器指针由外部管理）
AbstractMaterial::~AbstractMaterial() {}

// 应用材质（设置通用Uniform变量）
void AbstractMaterial::Use(mat4 & view, mat4 & proj, mat4 model) {
    shader->use(); // 激活着色器
    // 设置变换矩阵
    shader->setMat4("viewMat", view);
    shader->setMat4("projMat", proj);
    shader->setMat4("modelMat", model);
    // 设置材质参数
    shader->setFloat("material.shininess", shininess);
    shader->setVec3("material.color", color);
    shader->setBool("specular", specular);
    // 设置相机位置
    shader->setVec3("cameraPos", Setting::MainCamera->gameObject->transform()->position);
}

#pragma endregion


// ====================== StandandMaterial 标准材质 ======================
#pragma region StandardMaterial : AbstractMaterial

// 构造函数（设置组件名称）
StandandMaterial::StandandMaterial(Shader * shader, vec3 color, float skininess)
    : AbstractMaterial(shader, color, skininess) {
    name += "StandandMaterial"; // 设置材质名称
}

// 析构函数（无资源释放，留空）
StandandMaterial::~StandandMaterial() {}

// 应用材质（添加光照参数）
void StandandMaterial::Use(mat4 & view, mat4 & proj, mat4 model) {
    AbstractMaterial::Use(view, proj, model); // 调用基类实现
    for (auto light : *Setting::lights) // 遍历所有光照并添加到着色器
        shader->AddLight(light);
}

#pragma endregion


// ====================== BoxMaterial 盒子材质 ======================
#pragma region BoxMaterial : AbstractMaterial

// 构造函数（初始化纹理路径）
BoxMaterial::BoxMaterial(Shader * shader, string specularPath, string diffusePath, vec3 color, float skininess)
    : AbstractMaterial(shader, color, skininess), specularPath(specularPath), diffusePath(diffusePath) {}

// ImGui 调试界面（显示基类参数，无额外参数）
void BoxMaterial::OnGUI() const {
    AbstractMaterial::OnGUI(); // 显示基类的材质参数
}

// 应用材质（绑定纹理）
void BoxMaterial::Use(mat4 & view, mat4 & proj, mat4 model) {
    AbstractMaterial::Use(view, proj, model); // 调用基类实现
    // 设置纹理单元（假设纹理0为漫反射，纹理1为高光）
    shader->setInt("material.texture_diffuse0", TextureFromFile(diffusePath));
    shader->setInt("material.texture_specular0", TextureFromFile(specularPath));
    for (auto light : *Setting::lights) // 添加光照参数
        shader->AddLight(light);
}

// 析构函数（无资源释放，留空）
BoxMaterial::~BoxMaterial() {}

#pragma endregion


// ====================== ModelRender 模型渲染组件 ======================
#pragma region ModelRender

// ImGui 调试界面（显示关联的材质和模型）
void ModelRender::OnGUI() const {
    MonoBehavior::OnGUI(); // 显示基类的启用状态复选框
    material->OnGUI(); // 显示材质设置
    model->OnGUI(); // 显示模型信息
}

// 物理更新（渲染模型）
void ModelRender::RealUpdate() {
    MonoBehavior::RealUpdate();
    // 应用材质并绘制模型
    material->Use(viewMat, projMat, gameObject->transform()->GetModelMaterix());
    model->Draw(material->shader);
}

// 初始化（创建材质和模型）
void ModelRender::Start() {
    MonoBehavior::Start();
    material = new StandandMaterial(new Shader(shaderName)); // 创建标准材质
    model = new Model(workDir.substr(0, workDir.find_last_of('\\')) + "\\" + modelName); // 加载模型
}

// 构造函数（设置组件名称）
ModelRender::ModelRender() {
    name += "ModelRender"; // 设置组件名称
}

// 析构函数（释放材质和模型资源）
ModelRender::~ModelRender() {
    delete material;
    delete model;
}

#pragma endregion


// ====================== SkyboxRender 天空盒渲染组件 ======================
#pragma region SkyboxRender : MonoBehavior

// 天空盒顶点数据（立方体六个面）
vector<std::string> faces = { "right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg" }; // 天空盒纹理文件名称
float skyboxVertices[] = { /* 立方体顶点坐标（省略具体数值，用于生成天空盒几何体） */ };

// 初始化（创建天空盒几何体和纹理）
void SkyboxRender::Start() {
    MonoBehavior::Start();
    // 生成VAO和VBO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    // 设置顶点属性指针（仅位置）
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glActiveTexture(GL_TEXTURE0);
    this->vao = skyboxVAO; // 保存VAO句柄
    // 创建材质并设置天空盒纹理单元
    material = new StandandMaterial(new Shader("sky"));
    material->shader->setInt("skybox", 0);
}

// ImGui 调试界面（显示基类参数，无额外参数）
void SkyboxRender::OnGUI() const {
    MonoBehavior::OnGUI(); // 显示基类的启用状态复选框
}

// 物理更新（渲染天空盒）
void SkyboxRender::RealUpdate() {
    MonoBehavior::RealUpdate();
    material->Use(viewMat, projMat, mat4(mat3(gameObject->transform()->GetModelMaterix()))); // 忽略模型矩阵的平移（天空盒始终在原点）
    material->shader->setInt("skybox", 0); // 设置天空盒纹理单元
    glDepthMask(GL_FALSE); // 禁用深度写入（天空盒在最远平面）
    glBindVertexArray(vao); // 绑定天空盒VAO
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId); // 绑定立方体贴图
    glDrawArrays(GL_TRIANGLES, 0, 36); // 绘制天空盒
    glBindVertexArray(0); // 解绑
    glDepthMask(GL_TRUE); // 恢复深度写入
}

// 构造函数（加载天空盒纹理）
SkyboxRender::SkyboxRender() {
    name += "SkyboxRender"; // 设置组件名称
    textureId = loadCubemap(faces); // 加载立方体贴图
}

// 析构函数（无资源释放，纹理ID由外部管理）
SkyboxRender::~SkyboxRender() {}

#pragma endregion


// ====================== GameObject 游戏对象类 ======================
#pragma region GameObject : Object

// 模板方法：添加组件并启动（自动调用Start方法）
template<typename T>
T* GameObject::AddComponentStart() {
    var mb = (MonoBehavior*)new T(); // 创建组件实例
    mb->gameObject = this; // 设置所属游戏对象
    mb->Start(); // 调用初始化方法
    scripts->push_back(mb); // 添加到脚本列表
    return (T*)mb; // 返回组件指针
}

// 其他模板方法（添加组件不启动、添加现有组件、获取组件、移除组件）类似，逻辑一致，注释从略

// 获取Transform组件（通过类型查找）
Transform * GameObject::transform() const {
    return GetComponent<Transform>(); // 调用模板方法获取组件
}

// ImGui 调试界面（显示启用状态和定位按钮）
void GameObject::OnGUI() const {
    ImGui::Checkbox("Enable", (bool*)&enable); // 启用状态复选框
    if (ImGui::Button("Go to here")) { // 定位按钮
        if (Setting::MainCamera && Setting::MainCamera->gameObject->enable)
            Setting::MainCamera->gameObject->transform()->position = transform()->position; // 移动主相机到当前对象位置
    }
}

// 静态ID生成器（确保每个对象ID唯一）
static int idS = 0;

// 构造函数（根据类型创建游戏对象并添加组件）
GameObject::GameObject(string name, Type type) : Object("GameObject_" + name) {
    this->id = idS++; // 分配唯一ID
    Setting::gameObjects->push_back(this); // 添加到全局游戏对象列表
    scripts = new std::list<MonoBehavior*>(); // 初始化脚本列表
    AddComponentStart<Transform>(); // 添加Transform组件

    switch (type) {
        case Cameras: // 相机对象
            auto camera = AddComponentStart<Camera>();
            if (!Setting::MainCamera) { // 设置为主相机
                Setting::MainCamera = camera;
                AddComponentStart<CameraMove>(); // 添加相机移动脚本
                // 删除默认旋转脚本（示例逻辑）
                auto jb = AddComponentStart<Rotate>();
                delete jb;
                scripts->remove(jb);
            }
            break;
        // 其他类型（方向光、点光源、聚光灯、盒子、模型、空对象）的组件添加逻辑类似，注释从略
    }
}

// 析构函数（释放所有脚本组件资源）
GameObject::~GameObject() {
    for (auto x : *scripts)
        delete x; // 释放每个脚本组件
    delete scripts; // 释放脚本列表
}

#pragma endregion


// ====================== Setting 全局设置类 ======================
#pragma region Setting

// 静态成员初始化（全局光照列表、游戏对象列表、主相机等）
std::vector<AbstractLight*>* Setting::lights = nullptr;
std::list<GameObject*>* Setting::gameObjects = nullptr;
string const Setting::settingDir = workDir + "\\settings"; // 设置文件目录
Camera* Setting::MainCamera = nullptr; // 主相机指针
bool Setting::lockMouse = false; // 鼠标锁定状态
float Setting::deltaTime = 0.02f; // 帧间隔时间（默认0.02秒）
vec2 Setting::windowSize = vec2(1200, 1000); // 窗口初始尺寸

// 统计指定类型的光照数量
int Setting::LightCount(AbstractLight::LightType type) {
    int n = 0;
    for (auto x : *lights)
        if (x->Type() == type)
            n++; // 遍历计数
    return n;
}

// 初始化全局设置（创建光照和游戏对象列表）
void Setting::InitSettings() {
    pWindowSize = &windowSize; // 设置窗口尺寸指针
    lights = new std::vector<AbstractLight*>(); // 创建光照列表
    gameObjects = new std::list<GameObject*>(); // 创建游戏对象列表
}

#pragma endregion
