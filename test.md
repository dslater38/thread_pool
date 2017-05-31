<style scoped>
 td {
    border: 1px solid black;
    vertical-align : top;
 }
</style>

## Subscription Cases

For each ACM case, apply the following tests to a representative sample of symbols to cover all 
the different instrument types as well as combinations of instrument types. 

<table><tr><td>
1. Ideal Case
<ul>
  <li>Register</li>
  <li>WsConnect</li>
  <li>Subscribe</li>
  <li>Receive initial image</li>
  <li>Unsubscribe</li>
  <li>Wait for unsubscribe response</li>
  <li>Disconnect</li>
 </ul></td><td>
2. Early Disconnect
<ul>
  <li>Register</li>
  <li>WsConnect</li>
  <li>Subscribe</li>
  <li>Receive initial image</li>
  <li>Unsubscribe</li>
  <li>Immediately Disconnect</li>
 </ul></td><td>
3. No Unsubscribe
<ul>
  <li>Register</li>
  <li>WsConnect</li>
  <li>Subscribe</li>
  <li>Receive initial image</li>
  <li>Disconnect</li>
 </ul></td></tr><tr><td>
4. No Image
<ul>
  <li>Register</li>
  <li>WsConnect</li>
  <li>Subscribe</li>
  <li>Immediately Unsubscribe</li>
  <li>Wait for unsubscribe response</li>
  <li>Disconnect</li>
</ul></td><td>
5. No Image 2
<ul>
  <li>Register</li>
  <li>WsConnect</li>
  <li>Subscribe</li>
  <li>Immediately Unsubscribe</li>
  <li>Immediately Disconnect</li>
</ul></td><td>
6. No Image 3
<ul>
  <li>Register</li>
  <li>WsConnect</li>
  <li>Subscribe</li>
  <li>Immediately Disconnect</li>
</ul></td></tr>
</table>
