## 🔐 HMAC 详解

HMAC 是密码学中非常重要的概念，特别是在 API 安全、消息认证和数据完整性验证中广泛使用。让我为您详细解释。

---

## 一、HMAC 基本概念

### 定义
**HMAC (Hash-based Message Authentication Code)**
- 中文：**基于哈希的消息认证码**
- 本质：一种使用**密钥**和**哈希函数**来验证消息**完整性**和**真实性**的技术

### 通俗理解
```
HMAC = 哈希函数 + 密钥

就像给消息盖上一个"防伪印章"：
- 只有知道密钥的人才能生成正确的印章
- 任何对消息的篡改都会导致印章失效
- 接收方可以验证消息确实来自持有密钥的发送方
```

---

## 二、HMAC 的工作原理

### 数学公式
```
HMAC(K, M) = H((K ⊕ opad) || H((K ⊕ ipad) || M))

其中：
K     = 密钥（Secret Key）
M     = 消息（Message）
H     = 哈希函数（如 SHA-256）
⊕     = XOR 异或运算
||    = 拼接操作
opad  = 外部填充（0x5c 重复）
ipad  = 内部填充（0x36 重复）
```

### 简化理解的过程

```
步骤1: 内部哈希
   内部密钥 = K ⊕ ipad
   内部结果 = H(内部密钥 + 消息)

步骤2: 外部哈希
   外部密钥 = K ⊕ opad
   最终结果 = H(外部密钥 + 内部结果)
```

### 可视化流程

```
消息: "Hello World"
密钥: "my-secret-key"
         ↓
    ┌────────────┐
    │  步骤 1    │
    │ 密钥 ⊕ ipad│
    └──────┬─────┘
           ↓
    ┌────────────┐
    │  步骤 2    │
    │ + 消息     │
    └──────┬─────┘
           ↓
    ┌────────────┐
    │  步骤 3    │
    │ SHA-256    │
    └──────┬─────┘
           ↓
    内部哈希值
           ↓
    ┌────────────┐
    │  步骤 4    │
    │ 密钥 ⊕ opad│
    └──────┬─────┘
           ↓
    ┌────────────┐
    │  步骤 5    │
    │ + 内部哈希 │
    └──────┬─────┘
           ↓
    ┌────────────┐
    │  步骤 6    │
    │ SHA-256    │
    └──────┬─────┘
           ↓
    HMAC 结果
```

---

## 三、实际代码实现

### 1. **Node.js 实现**

```javascript
const crypto = require('crypto');

// 基础使用
function generateHMAC(message, secret) {
  return crypto
    .createHmac('sha256', secret)
    .update(message)
    .digest('hex');
}

// 示例
const message = 'Hello World';
const secret = 'my-secret-key';
const hmac = generateHMAC(message, secret);

console.log('消息:', message);
console.log('密钥:', secret);
console.log('HMAC:', hmac);
// 输出: a7b3c2d1e4f5... (64个十六进制字符)

// 验证 HMAC
function verifyHMAC(message, secret, receivedHmac) {
  const calculatedHmac = generateHMAC(message, secret);
  
  // 使用时间安全的比较，防止时序攻击
  return crypto.timingSafeEqual(
    Buffer.from(calculatedHmac),
    Buffer.from(receivedHmac)
  );
}

// 使用示例
const isValid = verifyHMAC(message, secret, hmac);
console.log('HMAC 验证:', isValid); // true
```

### 2. **Python 实现**

```python
import hmac
import hashlib

def generate_hmac(message, secret):
    """生成 HMAC"""
    return hmac.new(
        secret.encode('utf-8'),
        message.encode('utf-8'),
        hashlib.sha256
    ).hexdigest()

def verify_hmac(message, secret, received_hmac):
    """验证 HMAC（防时序攻击）"""
    calculated_hmac = generate_hmac(message, secret)
    return hmac.compare_digest(calculated_hmac, received_hmac)

# 示例
message = 'Hello World'
secret = 'my-secret-key'
hmac_value = generate_hmac(message, secret)

print(f'消息: {message}')
print(f'密钥: {secret}')
print(f'HMAC: {hmac_value}')

# 验证
is_valid = verify_hmac(message, secret, hmac_value)
print(f'HMAC 验证: {is_valid}')  # True
```

### 3. **Java 实现**

```java
import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.util.Base64;

public class HMACExample {
    
    public static String generateHMAC(String message, String secret) 
            throws NoSuchAlgorithmException, InvalidKeyException {
        
        Mac hmacSHA256 = Mac.getInstance("HmacSHA256");
        SecretKeySpec secretKey = new SecretKeySpec(
            secret.getBytes(), 
            "HmacSHA256"
        );
        
        hmacSHA256.init(secretKey);
        byte[] hmacBytes = hmacSHA256.doFinal(message.getBytes());
        
        // 转换为十六进制
        StringBuilder sb = new StringBuilder();
        for (byte b : hmacBytes) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }
    
    public static void main(String[] args) throws Exception {
        String message = "Hello World";
        String secret = "my-secret-key";
        
        String hmac = generateHMAC(message, secret);
        System.out.println("消息: " + message);
        System.out.println("密钥: " + secret);
        System.out.println("HMAC: " + hmac);
    }
}
```

### 4. **PHP 实现**

```php
<?php

function generateHMAC($message, $secret) {
    return hash_hmac('sha256', $message, $secret);
}

function verifyHMAC($message, $secret, $receivedHmac) {
    $calculatedHmac = generateHMAC($message, $secret);
    // 使用 hash_equals 防止时序攻击
    return hash_equals($calculatedHmac, $receivedHmac);
}

// 示例
$message = 'Hello World';
$secret = 'my-secret-key';
$hmac = generateHMAC($message, $secret);

echo "消息: $message\n";
echo "密钥: $secret\n";
echo "HMAC: $hmac\n";

// 验证
$isValid = verifyHMAC($message, $secret, $hmac);
echo "HMAC 验证: " . ($isValid ? 'true' : 'false') . "\n";
?>
```

---

## 四、实际应用场景

### 1. **API 签名认证** ⭐⭐⭐⭐⭐

#### 场景：保护 API 不被未授权访问和篡改

```javascript
// ========== 服务端 ==========
const express = require('express');
const crypto = require('crypto');
const app = express();

app.use(express.json());

// 用户的 API 密钥（存储在数据库中）
const API_KEYS = {
  'hesphoros': 'sk_live_a1b2c3d4e5f6g7h8'
};

// HMAC 验证中间件
const verifyHMAC = (req, res, next) => {
  const apiKey = req.headers['x-api-key'];
  const timestamp = req.headers['x-timestamp'];
  const receivedSignature = req.headers['x-signature'];
  
  // 验证密钥存在
  if (!apiKey || !API_KEYS[apiKey]) {
    return res.status(401).json({ error: '无效的 API Key' });
  }
  
  // 防止重放攻击（5分钟内有效）
  const now = Math.floor(Date.now() / 1000);
  if (Math.abs(now - timestamp) > 300) {
    return res.status(401).json({ error: '请求已过期' });
  }
  
  // 构造签名内容
  const method = req.method;
  const path = req.originalUrl;
  const body = JSON.stringify(req.body);
  const signatureContent = `${method}${path}${timestamp}${body}`;
  
  // 计算 HMAC
  const secret = API_KEYS[apiKey];
  const calculatedSignature = crypto
    .createHmac('sha256', secret)
    .update(signatureContent)
    .digest('hex');
  
  // 验证签名
  if (!crypto.timingSafeEqual(
    Buffer.from(calculatedSignature),
    Buffer.from(receivedSignature)
  )) {
    return res.status(401).json({ error: '签名验证失败' });
  }
  
  req.apiUser = apiKey;
  next();
};

// 受保护的 API 端点
app.post('/api/transfer', verifyHMAC, (req, res) => {
  const { from, to, amount } = req.body;
  
  console.log(`API 用户: ${req.apiUser}`);
  console.log(`转账: ${from} -> ${to}, 金额: ${amount}`);
  
  res.json({ 
    success: true,
    message: '转账成功',
    transactionId: 'tx_' + Date.now()
  });
});

app.listen(3000, () => {
  console.log('API 服务运行在 http://localhost:3000');
});
```

```javascript
// ========== 客户端 ==========
const crypto = require('crypto');
const axios = require('axios');

class APIClient {
  constructor(apiKey, apiSecret) {
    this.apiKey = apiKey;
    this.apiSecret = apiSecret;
    this.baseURL = 'http://localhost:3000';
  }
  
  generateSignature(method, path, timestamp, body) {
    const signatureContent = `${method}${path}${timestamp}${body}`;
    return crypto
      .createHmac('sha256', this.apiSecret)
      .update(signatureContent)
      .digest('hex');
  }
  
  async request(method, path, data = {}) {
    const timestamp = Math.floor(Date.now() / 1000);
    const body = JSON.stringify(data);
    const signature = this.generateSignature(method, path, timestamp, body);
    
    const response = await axios({
      method,
      url: this.baseURL + path,
      data: data,
      headers: {
        'Content-Type': 'application/json',
        'X-API-Key': this.apiKey,
        'X-Timestamp': timestamp,
        'X-Signature': signature
      }
    });
    
    return response.data;
  }
}

// 使用示例
const client = new APIClient('hesphoros', 'sk_live_a1b2c3d4e5f6g7h8');

client.request('POST', '/api/transfer', {
  from: 'account_123',
  to: 'account_456',
  amount: 1000
})
.then(result => {
  console.log('API 响应:', result);
})
.catch(error => {
  console.error('API 错误:', error.response?.data || error.message);
});
```

### 2. **Webhook 验证**

#### 场景：验证 Webhook 确实来自可信的第三方服务

```javascript
// ========== GitHub Webhook 示例 ==========
const express = require('express');
const crypto = require('crypto');
const app = express();

app.use(express.json());

const WEBHOOK_SECRET = 'your-webhook-secret';

// GitHub Webhook 处理
app.post('/webhook/github', (req, res) => {
  const signature = req.headers['x-hub-signature-256'];
  const body = JSON.stringify(req.body);
  
  // 计算 HMAC
  const expectedSignature = 'sha256=' + crypto
    .createHmac('sha256', WEBHOOK_SECRET)
    .update(body)
    .digest('hex');
  
  // 验证签名
  if (signature !== expectedSignature) {
    return res.status(401).send('Invalid signature');
  }
  
  // 处理 webhook 事件
  console.log('GitHub 事件:', req.body.action);
  res.status(200).send('Webhook received');
});

// ========== Stripe Webhook 示例 ==========
app.post('/webhook/stripe', (req, res) => {
  const signature = req.headers['stripe-signature'];
  const body = JSON.stringify(req.body);
  
  // Stripe 使用特殊的签名格式
  const stripe = require('stripe')('sk_test_...');
  
  try {
    const event = stripe.webhooks.constructEvent(
      body,
      signature,
      WEBHOOK_SECRET
    );
    
    console.log('Stripe 事件:', event.type);
    res.json({ received: true });
  } catch (err) {
    return res.status(400).send(`Webhook Error: ${err.message}`);
  }
});

app.listen(3000);
```

### 3. **JWT 签名**

HMAC 是 JWT 最常用的签名算法之一：

```javascript
// JWT 使用 HMAC-SHA256 签名
const jwt = require('jsonwebtoken');

const payload = {
  sub: 'hesphoros',
  name: 'Hesphoros',
  iat: Math.floor(Date.now() / 1000)
};

const secret = 'your-secret-key';

// 生成 JWT（内部使用 HMAC）
const token = jwt.sign(payload, secret, { algorithm: 'HS256' });
console.log('JWT Token:', token);

// JWT 的结构：
// Header.Payload.Signature
// Signature = HMAC-SHA256(Header + "." + Payload, secret)

// 验证 JWT
try {
  const decoded = jwt.verify(token, secret);
  console.log('验证成功:', decoded);
} catch (err) {
  console.error('验证失败:', err.message);
}
```

### 4. **Cookie 签名**

```javascript
const express = require('express');
const cookieParser = require('cookie-parser');
const app = express();

// 使用密钥签名 cookie
app.use(cookieParser('my-secret-key'));

app.get('/set-cookie', (req, res) => {
  // 设置签名 cookie
  res.cookie('user', 'hesphoros', { signed: true });
  res.send('Cookie 已设置');
});

app.get('/get-cookie', (req, res) => {
  // 读取并验证签名 cookie
  const user = req.signedCookies.user;
  
  if (user) {
    res.send(`用户: ${user}`);
  } else {
    res.send('Cookie 无效或已被篡改');
  }
});

app.listen(3000);
```

### 5. **消息队列消息认证**

```javascript
// 发送消息时签名
const crypto = require('crypto');

function sendMessage(queue, message, secret) {
  const timestamp = Date.now();
  const messageData = JSON.stringify({ message, timestamp });
  
  const signature = crypto
    .createHmac('sha256', secret)
    .update(messageData)
    .digest('hex');
  
  queue.send({
    data: messageData,
    signature: signature
  });
}

// 接收消息时验证
function receiveMessage(messageObj, secret) {
  const { data, signature } = messageObj;
  
  const calculatedSignature = crypto
    .createHmac('sha256', secret)
    .update(data)
    .digest('hex');
  
  if (signature !== calculatedSignature) {
    throw new Error('消息签名无效');
  }
  
  const { message, timestamp } = JSON.parse(data);
  
  // 检查时间戳防止重放攻击
  if (Date.now() - timestamp > 60000) { // 1分钟
    throw new Error('消息已过期');
  }
  
  return message;
}
```

### 6. **AWS 签名请求（AWS Signature Version 4）**

```javascript
const crypto = require('crypto');
const AWS = require('aws-sdk');

// AWS 使用 HMAC-SHA256 签名所有 API 请求
function signAWSRequest(method, url, headers, payload, credentials) {
  const date = new Date().toISOString().split('T')[0].replace(/-/g, '');
  const region = 'us-east-1';
  const service = 's3';
  
  // 步骤 1: 创建规范请求
  const canonicalRequest = `${method}\n${url}\n\n${headers}\n${payload}`;
  
  // 步骤 2: 创建待签名字符串
  const stringToSign = `AWS4-HMAC-SHA256\n${date}\n${region}/${service}/aws4_request\n${canonicalRequest}`;
  
  // 步骤 3: 计算签名（多次 HMAC）
  const kDate = crypto.createHmac('sha256', 'AWS4' + credentials.secretKey).update(date).digest();
  const kRegion = crypto.createHmac('sha256', kDate).update(region).digest();
  const kService = crypto.createHmac('sha256', kRegion).update(service).digest();
  const kSigning = crypto.createHmac('sha256', kService).update('aws4_request').digest();
  const signature = crypto.createHmac('sha256', kSigning).update(stringToSign).digest('hex');
  
  return signature;
}
```

---

## 五、HMAC vs 其他技术对比

### 1. **HMAC vs 普通哈希**

```javascript
// 普通哈希（不安全）
const hash = crypto.createHash('sha256').update('Hello').digest('hex');
// 问题：任何人都能计算相同的哈希值

// HMAC（安全）
const hmac = crypto.createHmac('sha256', 'secret').update('Hello').digest('hex');
// 优势：需要知道密钥才能生成正确的 HMAC
```

| 特性 | 普通哈希 | HMAC |
|------|---------|------|
| **需要密钥** | ❌ 否 | ✅ 是 |
| **防篡改** | ✅ 是 | ✅ 是 |
| **身份验证** | ❌ 否 | ✅ 是 |
| **防重放攻击** | ❌ 否 | ⚠️ 需配合时间戳 |
| **用途** | 数据指纹 | 消息认证 |

### 2. **HMAC vs 数字签名**

```javascript
// HMAC（对称密钥）
const hmac = crypto.createHmac('sha256', 'shared-secret').update(data).digest('hex');

// 数字签名（非对称密钥）
const { privateKey, publicKey } = crypto.generateKeyPairSync('rsa', {
  modulusLength: 2048
});

const sign = crypto.createSign('SHA256');
sign.update(data);
const signature = sign.sign(privateKey, 'hex');

// 验证
const verify = crypto.createVerify('SHA256');
verify.update(data);
const isValid = verify.verify(publicKey, signature, 'hex');
```

| 特性 | HMAC | 数字签名 |
|------|------|---------|
| **密钥类型** | 对称（共享密钥） | 非对称（公私钥对） |
| **性能** | ⭐⭐⭐⭐⭐ 快 | ⭐⭐⭐ 慢 |
| **安全性** | ⭐⭐⭐⭐ 高 | ⭐⭐⭐⭐⭐ 更高 |
| **密钥分发** | ⚠️ 困难 | ✅ 容易（公钥可公开） |
| **不可否认性** | ❌ 否 | ✅ 是 |
| **使用场景** | API 认证、内部系统 | 证书、区块链、法律文档 |

### 3. **不同哈希算法的 HMAC**

```javascript
const message = 'Hello World';
const secret = 'my-secret';

// HMAC-MD5（不推荐，已不安全）
const hmacMD5 = crypto.createHmac('md5', secret).update(message).digest('hex');

// HMAC-SHA1（不推荐用于新系统）
const hmacSHA1 = crypto.createHmac('sha1', secret).update(message).digest('hex');

// HMAC-SHA256（推荐）⭐⭐⭐⭐⭐
const hmacSHA256 = crypto.createHmac('sha256', secret).update(message).digest('hex');

// HMAC-SHA512（更强安全性）
const hmacSHA512 = crypto.createHmac('sha512', secret).update(message).digest('hex');

console.log('MD5:    ', hmacMD5);     // 32 字符
console.log('SHA1:   ', hmacSHA1);    // 40 字符
console.log('SHA256: ', hmacSHA256);  // 64 字符
console.log('SHA512: ', hmacSHA512);  // 128 字符
```

---

## 六、HMAC 安全最佳实践

### 1. **密钥管理**

```javascript
// ❌ 不要硬编码密钥
const secret = 'my-secret-key';

// ✅ 使用环境变量
require('dotenv').config();
const secret = process.env.HMAC_SECRET;

// ✅ 使用密钥管理服务
const AWS = require('aws-sdk');
const secretsManager = new AWS.SecretsManager();
const secret = await secretsManager.getSecretValue({ SecretId: 'api-hmac-key' }).promise();

// ✅ 定期轮换密钥
const secrets = {
  current: process.env.HMAC_SECRET_CURRENT,
  previous: process.env.HMAC_SECRET_PREVIOUS
};

function verifyWithRotation(message, signature) {
  // 先用当前密钥验证
  if (verifyHMAC(message, secrets.current, signature)) {
    return true;
  }
  // 如果失败，尝试旧密钥
  return verifyHMAC(message, secrets.previous, signature);
}
```

### 2. **防止时序攻击**

```javascript
// ❌ 不安全的比较
if (calculatedHmac === receivedHmac) {
  // 字符串比较会在第一个不同字符处短路返回
  // 攻击者可通过测量响应时间推断密钥
}

// ✅ 时间安全的比较
const crypto = require('crypto');

function secureCompare(a, b) {
  return crypto.timingSafeEqual(
    Buffer.from(a),
    Buffer.from(b)
  );
}

// 或使用专门的库
const timingSafeCompare = require('tsscmp');
if (timingSafeCompare(calculatedHmac, receivedHmac)) {
  // 安全
}
```

### 3. **防止重放攻击**

```javascript
// 使用时间戳 + Nonce
const redis = require('redis');
const client = redis.createClient();

async function verifyRequest(req) {
  const { timestamp, nonce, signature } = req.headers;
  
  // 1. 检查时间戳（5分钟内有效）
  const now = Math.floor(Date.now() / 1000);
  if (Math.abs(now - timestamp) > 300) {
    throw new Error('请求已过期');
  }
  
  // 2. 检查 nonce 是否已使用
  const nonceKey = `nonce:${nonce}`;
  const exists = await client.exists(nonceKey);
  if (exists) {
    throw new Error('重放攻击检测');
  }
  
  // 3. 验证 HMAC
  const content = `${timestamp}${nonce}${req.body}`;
  const calculatedSignature = generateHMAC(content, SECRET);
  
  if (!secureCompare(calculatedSignature, signature)) {
    throw new Error('签名无效');
  }
  
  // 4. 记录 nonce（TTL 5分钟）
  await client.setex(nonceKey, 300, '1');
  
  return true;
}
```

### 4. **选择合适的哈希算法**

```javascript
// 2025年推荐使用：
// ✅ SHA-256（平衡性能和安全）- 推荐
// ✅ SHA-512（高安全性场景）
// ✅ SHA-3（最新标准）

// ❌ 避免使用：
// MD5 - 已被破解
// SHA-1 - 不再安全

const hmac = crypto.createHmac('sha256', secret); // ✅
```

### 5. **密钥长度**

```javascript
// 密钥长度建议：
// - 至少与哈希输出长度相同
// - SHA-256: 至少 32 字节（256 位）
// - SHA-512: 至少 64 字节（512 位）

// 生成强密钥
const crypto = require('crypto');
const secret = crypto.randomBytes(32).toString('hex');
console.log('强密钥:', secret);
// 输出: a1b2c3d4e5f6... (64个十六进制字符 = 32字节)
```

---

## 七、实战练习

### 练习1：构建一个签名 API 客户端

```javascript
class SignedAPIClient {
  constructor(apiKey, apiSecret) {
    this.apiKey = apiKey;
    this.apiSecret = apiSecret;
  }
  
  generateNonce() {
    return crypto.randomBytes(16).toString('hex');
  }
  
  sign(method, path, params = {}) {
    const timestamp = Math.floor(Date.now() / 1000);
    const nonce = this.generateNonce();
    const queryString = new URLSearchParams(params).toString();
    
    // 签名内容：方法 + 路径 + 时间戳 + nonce + 参数
    const signatureContent = `${method}${path}${timestamp}${nonce}${queryString}`;
    
    const signature = crypto
      .createHmac('sha256', this.apiSecret)
      .update(signatureContent)
      .digest('hex');
    
    return {
      timestamp,
      nonce,
      signature,
      apiKey: this.apiKey
    };
  }
  
  async request(method, path, params = {}) {
    const authData = this.sign(method, path, params);
    
    const response = await fetch(`https://api.example.com${path}?${new URLSearchParams(params)}`, {
      method,
      headers: {
        'X-API-Key': authData.apiKey,
        'X-Timestamp': authData.timestamp,
        'X-Nonce': authData.nonce,
        'X-Signature': authData.signature
      }
    });
    
    return response.json();
  }
}

// 使用
const client = new SignedAPIClient('hesphoros', 'sk_live_abc123');
const data = await client.request('GET', '/api/balance', { currency: 'USD' });
```

### 练习2：实现 GitHub Webhook 验证

```javascript
const express = require('express');
const crypto = require('crypto');
const app = express();

// 保留原始 body（重要！）
app.use(express.json({
  verify: (req, res, buf) => {
    req.rawBody = buf.toString('utf8');
  }
}));

const WEBHOOK_SECRET = 'your-webhook-secret';

app.post('/webhook', (req, res) => {
  const signature = req.headers['x-hub-signature-256'];
  
  if (!signature) {
    return res.status(401).send('Missing signature');
  }
  
  // GitHub 使用 sha256= 前缀
  const expectedSignature = 'sha256=' + crypto
    .createHmac('sha256', WEBHOOK_SECRET)
    .update(req.rawBody)
    .digest('hex');
  
  // 时间安全比较
  if (!crypto.timingSafeEqual(Buffer.from(signature), Buffer.from(expectedSignature))) {
    return res.status(401).send('Invalid signature');
  }
  
  // 处理 webhook
  console.log('Event:', req.body.action);
  res.status(200).send('Success');
});

app.listen(3000);
```

---

## 八、常见问题

### Q1: HMAC 可以解密吗？

```
❌ 不能！HMAC 是单向的。

HMAC 是哈希函数，不是加密：
- 加密：可以解密还原原文
- HMAC：只能验证，无法还原

错误理解：HMAC("Hello") → 解密 → "Hello" ❌
正确理解：HMAC("Hello") → 用于验证 "Hello" 是否被篡改 ✅
```

### Q2: 为什么不直接用哈希，要用 HMAC？

```javascript
// 场景：API 请求签名

// ❌ 只用哈希（不安全）
const hash = crypto.createHash('sha256')
  .update(message)
  .digest('hex');

// 问题：任何人都能计算相同的哈希
// 攻击者可以修改消息，重新计算哈希

// ✅ 使用 HMAC（安全）
const hmac = crypto.createHmac('sha256', SECRET_KEY)
  .update(message)
  .digest('hex');

// 优势：只有知道密钥的人才能生成正确的 HMAC
```

### Q3: HMAC 的密钥需要加密吗？

```
密钥本身不需要加密，但需要：
✅ 安全存储（环境变量、密钥管理服务）
✅ 安全传输（HTTPS、加密通道）
✅ 访问控制（最小权限原则）
❌ 不要硬编码在代码中
❌ 不要提交到版本控制
```

### Q4: HMAC 能防止什么攻击？

```
✅ 防篡改：消息被修改会导致 HMAC 验证失败
✅ 防伪造：没有密钥无法生成有效的 HMAC
⚠️ 防重放：需配合时间戳/nonce
❌ 不防窃听：HMAC 不加密消息内容（需配合 HTTPS）
❌ 不防密钥泄露：密钥泄露则失去所有保护
```

---
