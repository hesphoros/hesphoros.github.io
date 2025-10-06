## 🎫 Token 详解

Token 是一个广泛使用的概念，在不同的技术领域有不同的含义

---

## 一、Token 的基本概念

### 定义
**Token（令牌）** 是一个用于**身份验证、授权或数据传递**的字符串，通常包含经过编码的信息。可以理解为：
- 🎟️ **数字门票** - 证明你有权访问某些资源
- 🔑 **电子钥匙** - 打开特定服务的凭证
- 📇 **身份证明** - 表明你是谁

---

## 二、Token 的主要类型

### 1. **访问令牌（Access Token）** ⭐⭐⭐⭐⭐

用于访问受保护的资源或 API。

#### JWT (JSON Web Token) - 最流行

**结构：**
```
Header.Payload.Signature
```

**实际示例：**
```
eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c
```

**解码后：**

```json
// Header
{
  "alg": "HS256",
  "typ": "JWT"
}

// Payload
{
  "sub": "1234567890",
  "name": "John Doe",
  "email": "john@example.com",
  "role": "admin",
  "iat": 1516239022,    // 签发时间
  "exp": 1516242622     // 过期时间
}

// Signature
HMACSHA256(
  base64UrlEncode(header) + "." +
  base64UrlEncode(payload),
  secret
)
```

#### JWT 实际应用

**Node.js 示例：**
```javascript
const jwt = require('jsonwebtoken');

// 1. 生成 Token
const generateToken = (userId) => {
  const payload = {
    sub: userId,
    name: 'hesphoros',
    role: 'developer',
    iat: Math.floor(Date.now() / 1000)
  };
  
  const secret = 'your-secret-key-keep-it-safe';
  const options = {
    expiresIn: '24h'  // 24小时后过期
  };
  
  return jwt.sign(payload, secret, options);
};

// 生成 token
const token = generateToken('user123');
console.log('Token:', token);

// 2. 验证 Token
const verifyToken = (token) => {
  try {
    const decoded = jwt.verify(token, 'your-secret-key-keep-it-safe');
    console.log('Token 有效:', decoded);
    return decoded;
  } catch (error) {
    console.error('Token 无效:', error.message);
    return null;
  }
};

// 3. 在 Express 中间件中使用
const express = require('express');
const app = express();

// 认证中间件
const authenticateToken = (req, res, next) => {
  // 从请求头获取 token
  const authHeader = req.headers['authorization'];
  const token = authHeader && authHeader.split(' ')[1]; // Bearer TOKEN
  
  if (!token) {
    return res.status(401).json({ error: '缺少访问令牌' });
  }
  
  jwt.verify(token, 'your-secret-key-keep-it-safe', (err, user) => {
    if (err) {
      return res.status(403).json({ error: '令牌无效或已过期' });
    }
    req.user = user;
    next();
  });
};

// 受保护的路由
app.get('/api/protected', authenticateToken, (req, res) => {
  res.json({ 
    message: '这是受保护的数据',
    user: req.user 
  });
});

// 登录路由（生成 token）
app.post('/api/login', (req, res) => {
  // 验证用户名密码（示例简化）
  const { username, password } = req.body;
  
  if (username === 'hesphoros' && password === 'password123') {
    const token = generateToken(username);
    res.json({ 
      message: '登录成功',
      token: token 
    });
  } else {
    res.status(401).json({ error: '用户名或密码错误' });
  }
});

app.listen(3000);
```

**前端使用示例：**
```javascript
// 登录并保存 token
async function login(username, password) {
  const response = await fetch('http://localhost:3000/api/login', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({ username, password })
  });
  
  const data = await response.json();
  
  // 保存 token 到 localStorage
  localStorage.setItem('token', data.token);
  console.log('登录成功，Token 已保存');
}

// 使用 token 访问受保护的 API
async function fetchProtectedData() {
  const token = localStorage.getItem('token');
  
  const response = await fetch('http://localhost:3000/api/protected', {
    headers: {
      'Authorization': `Bearer ${token}`
    }
  });
  
  if (response.ok) {
    const data = await response.json();
    console.log('受保护的数据:', data);
  } else {
    console.error('访问被拒绝');
  }
}

// 登出
function logout() {
  localStorage.removeItem('token');
  console.log('已登出');
}
```

---

### 2. **刷新令牌（Refresh Token）**

用于获取新的访问令牌，通常有效期更长。

**工作流程：**
```
1. 用户登录 → 返回 Access Token (短期) + Refresh Token (长期)
2. 使用 Access Token 访问 API
3. Access Token 过期 → 使用 Refresh Token 获取新的 Access Token
4. Refresh Token 过期 → 需要重新登录
```

**实现示例：**
```javascript
// 生成双令牌
const generateTokens = (userId) => {
  const accessToken = jwt.sign(
    { sub: userId, type: 'access' },
    'access-secret',
    { expiresIn: '15m' }  // 15分钟
  );
  
  const refreshToken = jwt.sign(
    { sub: userId, type: 'refresh' },
    'refresh-secret',
    { expiresIn: '7d' }   // 7天
  );
  
  return { accessToken, refreshToken };
};

// 刷新令牌端点
app.post('/api/refresh', (req, res) => {
  const { refreshToken } = req.body;
  
  if (!refreshToken) {
    return res.status(401).json({ error: '缺少刷新令牌' });
  }
  
  jwt.verify(refreshToken, 'refresh-secret', (err, user) => {
    if (err) {
      return res.status(403).json({ error: '刷新令牌无效' });
    }
    
    // 生成新的访问令牌
    const newAccessToken = jwt.sign(
      { sub: user.sub, type: 'access' },
      'access-secret',
      { expiresIn: '15m' }
    );
    
    res.json({ accessToken: newAccessToken });
  });
});
```

**前端自动刷新：**
```javascript
// Axios 拦截器自动刷新 token
import axios from 'axios';

axios.interceptors.response.use(
  response => response,
  async error => {
    const originalRequest = error.config;
    
    // 如果是 401 错误且未重试过
    if (error.response.status === 401 && !originalRequest._retry) {
      originalRequest._retry = true;
      
      try {
        const refreshToken = localStorage.getItem('refreshToken');
        const response = await axios.post('/api/refresh', { refreshToken });
        
        const { accessToken } = response.data;
        localStorage.setItem('token', accessToken);
        
        // 重新发送原请求
        originalRequest.headers['Authorization'] = `Bearer ${accessToken}`;
        return axios(originalRequest);
      } catch (refreshError) {
        // 刷新失败，跳转到登录页
        window.location.href = '/login';
        return Promise.reject(refreshError);
      }
    }
    
    return Promise.reject(error);
  }
);
```

---

### 3. **OAuth Token（第三方登录）**

用于授权第三方应用访问用户资源。

**OAuth 2.0 流程：**
```
1. 用户点击"使用 GitHub 登录"
2. 跳转到 GitHub 授权页面
3. 用户同意授权
4. GitHub 返回 Authorization Code
5. 应用用 Code 换取 Access Token
6. 使用 Token 访问用户信息
```

**GitHub OAuth 示例：**
```javascript
// 1. 重定向到 GitHub 授权页面
app.get('/auth/github', (req, res) => {
  const clientId = 'your-github-client-id';
  const redirectUri = 'http://localhost:3000/auth/github/callback';
  const scope = 'user:email';
  
  const authUrl = `https://github.com/login/oauth/authorize?client_id=${clientId}&redirect_uri=${redirectUri}&scope=${scope}`;
  
  res.redirect(authUrl);
});

// 2. GitHub 回调处理
app.get('/auth/github/callback', async (req, res) => {
  const { code } = req.query;
  
  // 用 code 换取 access token
  const tokenResponse = await axios.post('https://github.com/login/oauth/access_token', {
    client_id: 'your-github-client-id',
    client_secret: 'your-github-client-secret',
    code: code
  }, {
    headers: { 'Accept': 'application/json' }
  });
  
  const accessToken = tokenResponse.data.access_token;
  
  // 使用 access token 获取用户信息
  const userResponse = await axios.get('https://api.github.com/user', {
    headers: { 'Authorization': `Bearer ${accessToken}` }
  });
  
  const user = userResponse.data;
  
  // 创建自己的 JWT token
  const myToken = jwt.sign({ githubId: user.id, username: user.login }, 'my-secret');
  
  res.json({ token: myToken, user });
});
```

---

### 4. **API Token（API 密钥）**

用于程序间认证，通常是长期有效的随机字符串。

**生成和使用：**
```javascript
const crypto = require('crypto');

// 生成 API Token
const generateApiToken = () => {
  return crypto.randomBytes(32).toString('hex');
};

// 存储到数据库
const apiToken = generateApiToken();
// 示例: a3f8d9e2b1c4567890abcdef12345678...

// 使用 API Token
app.get('/api/data', (req, res) => {
  const apiKey = req.headers['x-api-key'];
  
  // 在数据库中验证
  if (apiKey === 'a3f8d9e2b1c4567890abcdef12345678...') {
    res.json({ data: 'Protected data' });
  } else {
    res.status(401).json({ error: 'Invalid API key' });
  }
});
```

**使用示例：**
```bash
# cURL 请求
curl -H "X-API-Key: a3f8d9e2b1c4567890abcdef12345678..." \
  https://api.example.com/data

# JavaScript
fetch('https://api.example.com/data', {
  headers: {
    'X-API-Key': 'a3f8d9e2b1c4567890abcdef12345678...'
  }
});
```

---

### 5. **CSRF Token（跨站请求伪造防护）**

防止恶意网站代表用户执行操作。

**实现：**
```javascript
const csrf = require('csurf');
const express = require('express');
const app = express();

// 启用 CSRF 保护
const csrfProtection = csrf({ cookie: true });

// 渲染表单时生成 CSRF token
app.get('/form', csrfProtection, (req, res) => {
  res.render('form', { csrfToken: req.csrfToken() });
});

// 提交表单时验证
app.post('/submit', csrfProtection, (req, res) => {
  // CSRF token 自动验证
  res.send('表单提交成功');
});
```

**HTML 表单：**
```html
<form method="POST" action="/submit">
  <input type="hidden" name="_csrf" value="<%= csrfToken %>">
  <input type="text" name="data">
  <button type="submit">提交</button>
</form>
```

---

## 三、Token vs Session 对比

| 特性        | Token（令牌）                | Session（会话）   |
| --------- | ------------------------ | ------------- |
| **存储位置**  | 客户端（LocalStorage/Cookie） | 服务端（内存/数据库）   |
| **可扩展性**  | ⭐⭐⭐⭐⭐ 优秀（无状态）            | ⭐⭐ 较差（需要共享状态） |
| **安全性**   | ⭐⭐⭐ 中等（需防 XSS）           | ⭐⭐⭐⭐ 较好       |
| **跨域支持**  | ⭐⭐⭐⭐⭐ 优秀                 | ⭐⭐ 有限制        |
| **服务器负载** | ⭐⭐⭐⭐⭐ 低（无状态）             | ⭐⭐ 高（需存储）     |
| **移动端友好** | ⭐⭐⭐⭐⭐ 优秀                 | ⭐⭐⭐ 一般        |
| **撤销能力**  | ⭐⭐ 困难（需黑名单）              | ⭐⭐⭐⭐⭐ 容易      |

**工作流程对比：**

```
🎫 Token 认证流程：
1. 用户登录 → 服务器生成 Token → 返回给客户端
2. 客户端存储 Token（LocalStorage）
3. 每次请求携带 Token（Header: Authorization: Bearer xxx）
4. 服务器验证 Token 签名 → 解析用户信息 → 处理请求

📝 Session 认证流程：
1. 用户登录 → 服务器创建 Session → 返回 Session ID（Cookie）
2. 客户端自动存储 Session ID（Cookie）
3. 每次请求自动携带 Cookie
4. 服务器查找 Session → 获取用户信息 → 处理请求
```

---

## 四、Token 安全最佳实践

### 1. **存储安全**

```javascript
// ❌ 不安全 - 容易受 XSS 攻击
localStorage.setItem('token', token);

// ✅ 更安全 - HttpOnly Cookie
res.cookie('token', token, {
  httpOnly: true,   // 防止 JavaScript 访问
  secure: true,     // 只在 HTTPS 传输
  sameSite: 'strict', // 防止 CSRF
  maxAge: 24 * 60 * 60 * 1000 // 24小时
});

// ✅ 最安全 - HttpOnly Cookie + 刷新机制
// Access Token 存 Memory（短期）
// Refresh Token 存 HttpOnly Cookie（长期）
```

### 2. **传输安全**

```javascript
// ✅ 始终使用 HTTPS
// ✅ 在 Header 中传输，不要在 URL
const headers = {
  'Authorization': `Bearer ${token}`
};

// ❌ 不要这样做
const url = `https://api.example.com/data?token=${token}`;
```

### 3. **Token 过期和刷新**

```javascript
// ✅ 设置合理的过期时间
const accessToken = jwt.sign(payload, secret, { expiresIn: '15m' });
const refreshToken = jwt.sign(payload, secret, { expiresIn: '7d' });

// ✅ 实现 Token 刷新机制
// ✅ 使用黑名单机制撤销 Token（Redis）
```

### 4. **防止 Token 泄露**

```javascript
// ✅ 限制 Token 范围（Scope）
const token = jwt.sign({
  sub: userId,
  scope: 'read:profile write:posts'  // 最小权限原则
}, secret);

// ✅ 绑定 IP 或设备（可选）
const token = jwt.sign({
  sub: userId,
  ip: req.ip,
  device: req.headers['user-agent']
}, secret);

// ✅ 监控异常活动
if (decoded.ip !== req.ip) {
  // 记录可疑活动，要求重新认证
}
```

---

## 五、实际应用场景

### 1. **单页应用（SPA）认证**

```javascript
// React 示例
import React, { createContext, useState, useContext } from 'react';

const AuthContext = createContext();

export const AuthProvider = ({ children }) => {
  const [token, setToken] = useState(localStorage.getItem('token'));
  
  const login = async (username, password) => {
    const response = await fetch('/api/login', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ username, password })
    });
    
    const data = await response.json();
    setToken(data.token);
    localStorage.setItem('token', data.token);
  };
  
  const logout = () => {
    setToken(null);
    localStorage.removeItem('token');
  };
  
  const fetchWithAuth = async (url, options = {}) => {
    return fetch(url, {
      ...options,
      headers: {
        ...options.headers,
        'Authorization': `Bearer ${token}`
      }
    });
  };
  
  return (
    <AuthContext.Provider value={{ token, login, logout, fetchWithAuth }}>
      {children}
    </AuthContext.Provider>
  );
};

export const useAuth = () => useContext(AuthContext);

// 使用
function ProfilePage() {
  const { fetchWithAuth } = useAuth();
  const [profile, setProfile] = useState(null);
  
  useEffect(() => {
    fetchWithAuth('/api/profile')
      .then(res => res.json())
      .then(data => setProfile(data));
  }, []);
  
  return <div>{profile?.name}</div>;
}
```

### 2. **微服务间认证**

```javascript
// Service A 调用 Service B
const axios = require('axios');

const serviceToken = jwt.sign({
  service: 'service-a',
  scope: 'service:read service:write'
}, 'service-secret', { expiresIn: '1h' });

const response = await axios.get('http://service-b/api/data', {
  headers: {
    'Authorization': `Bearer ${serviceToken}`
  }
});
```

### 3. **移动应用认证**

```swift
// iOS Swift 示例
class AuthManager {
    static let shared = AuthManager()
    
    func login(username: String, password: String, completion: @escaping (Result<String, Error>) -> Void) {
        let url = URL(string: "https://api.example.com/login")!
        var request = URLRequest(url: url)
        request.httpMethod = "POST"
        request.setValue("application/json", forHTTPHeaderField: "Content-Type")
        
        let body = ["username": username, "password": password]
        request.httpBody = try? JSONSerialization.data(withJSONObject: body)
        
        URLSession.shared.dataTask(with: request) { data, response, error in
            guard let data = data else { return }
            
            if let json = try? JSONSerialization.jsonObject(with: data) as? [String: Any],
               let token = json["token"] as? String {
                // 保存到 Keychain
                KeychainHelper.save(token: token)
                completion(.success(token))
            }
        }.resume()
    }
    
    func makeAuthenticatedRequest(url: URL, completion: @escaping (Data?) -> Void) {
        guard let token = KeychainHelper.getToken() else { return }
        
        var request = URLRequest(url: url)
        request.setValue("Bearer \(token)", forHTTPHeaderField: "Authorization")
        
        URLSession.shared.dataTask(with: request) { data, response, error in
            completion(data)
        }.resume()
    }
}
```

---

## 六、常见问题

### Q1: Token 被盗怎么办？

```javascript
// 解决方案：
// 1. 使用短期 Token
// 2. 实现 Token 黑名单（Redis）
const redis = require('redis');
const client = redis.createClient();

// 撤销 Token
const revokeToken = async (token) => {
  const decoded = jwt.decode(token);
  const expiresIn = decoded.exp - Math.floor(Date.now() / 1000);
  await client.setex(`blacklist:${token}`, expiresIn, 'revoked');
};

// 验证时检查黑名单
const isTokenRevoked = async (token) => {
  return await client.exists(`blacklist:${token}`);
};
```

### Q2: 如何实现"记住我"功能？

```javascript
// 使用 Refresh Token
app.post('/login', (req, res) => {
  const { username, password, rememberMe } = req.body;
  
  const accessToken = jwt.sign({ sub: username }, 'secret', { expiresIn: '15m' });
  
  const refreshToken = jwt.sign({ sub: username }, 'refresh-secret', {
    expiresIn: rememberMe ? '30d' : '7d'
  });
  
  res.json({ accessToken, refreshToken });
});
```

### Q3: 如何处理多设备登录？

```javascript
// 方案1: 允许多设备，每个设备独立 Token
// 方案2: 限制设备数量
const userSessions = new Map(); // userId -> [token1, token2, ...]

const addSession = (userId, token) => {
  const sessions = userSessions.get(userId) || [];
  if (sessions.length >= 3) {
    sessions.shift(); // 移除最老的会话
  }
  sessions.push(token);
  userSessions.set(userId, sessions);
};
```

---

	