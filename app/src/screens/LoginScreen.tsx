import React, { useMemo, useState } from "react";
import { View, Text, TextInput, TouchableOpacity, StyleSheet, ScrollView, Alert, ActivityIndicator } from "react-native";
import { useTheme, Palette } from "../theme";
import { saveAuth, Auth } from "../state/auth";
import { loginWithPassword } from "../cloud/login";

// Aqara federates login but binds your devices to ONE data centre — you only see
// your locks when you sign in to the region the account actually lives in.
const REGIONS: { key: string; label: string }[] = [
  { key: "SEA", label: "Asia / VN" },
  { key: "CN", label: "China" },
  { key: "US", label: "USA" },
  { key: "EU", label: "Europe" },
  { key: "KR", label: "Korea" },
];

export default function LoginScreen({ onDone }: { onDone: (a: Auth) => void }) {
  const { t, dark, toggle } = useTheme();
  const s = useMemo(() => makeStyles(t), [t]);
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  const [showPwd, setShowPwd] = useState(false);
  const [area, setArea] = useState("SEA");
  const [busy, setBusy] = useState(false);

  async function doPasswordLogin() {
    setBusy(true);
    try {
      const a = await loginWithPassword(email.trim(), password, area);
      await saveAuth(a); onDone(a);
    } catch (e: any) {
      Alert.alert("Sign-in failed", String(e?.message ?? e));
    } finally { setBusy(false); }
  }

  return (
    <ScrollView contentContainerStyle={s.c} keyboardShouldPersistTaps="handled">
      <View style={s.topRow}>
        <Text style={s.title}>Aquara Local</Text>
        <TouchableOpacity style={s.themeToggle} onPress={toggle} hitSlop={{ top: 10, bottom: 10, left: 10, right: 10 }}>
          <Text style={[s.themeIcon, { color: t.text }]}>{dark ? "☀︎" : "☾"}</Text>
        </TouchableOpacity>
      </View>

      <Text style={s.label}>Email</Text>
      <TextInput style={s.input} placeholderTextColor={t.faint} autoCapitalize="none" keyboardType="email-address"
        value={email} onChangeText={setEmail} placeholder="email@gmail.com" />

      <Text style={s.label}>Password</Text>
      <View style={s.pwdWrap}>
        <TextInput style={s.pwdInput} placeholderTextColor={t.faint} secureTextEntry={!showPwd}
          value={password} onChangeText={setPassword} placeholder="••••••••" />
        <TouchableOpacity style={s.eyeBtn} onPress={() => setShowPwd((v) => !v)} hitSlop={{ top: 8, bottom: 8, left: 8, right: 8 }}>
          <Text style={[s.eye, { color: t.sub }]}>{showPwd ? "⧸" : "◉"}</Text>
        </TouchableOpacity>
      </View>

      <Text style={s.label}>Region</Text>
      <View style={s.regionRow}>
        {REGIONS.map((r) => (
          <TouchableOpacity key={r.key} style={[s.regionChip, area === r.key && s.regionChipOn]} onPress={() => setArea(r.key)}>
            <Text style={[s.regionChipT, area === r.key && s.regionChipTOn]}>{r.label}</Text>
          </TouchableOpacity>
        ))}
      </View>
      <Text style={s.regionHint}>Pick the region your account is registered in (China-mainland accounts → “China”).</Text>

      <TouchableOpacity style={[s.btn, busy && { opacity: 0.6 }]} disabled={busy} onPress={doPasswordLogin}>
        {busy ? <ActivityIndicator color="#fff" /> : <Text style={s.btnT}>Sign in</Text>}
      </TouchableOpacity>

      <Text style={s.note}>Sign in with your Aqara email/password (pure-JS RSA + sign, no official app needed).</Text>
    </ScrollView>
  );
}
const makeStyles = (t: Palette) => StyleSheet.create({
  c: { padding: 24, paddingTop: 70, flexGrow: 1 },
  topRow: { alignItems: "center", justifyContent: "center", marginBottom: 28, minHeight: 34 },
  title: { fontSize: 30, fontWeight: "800", color: t.text, textAlign: "center" },
  themeToggle: { position: "absolute", right: 0, top: 4 },
  label: { fontSize: 13, color: t.sub, marginTop: 14, marginBottom: 6, fontWeight: "600" },
  input: { borderWidth: 1, borderColor: t.border, borderRadius: 10, padding: 13, fontSize: 16, backgroundColor: t.inputBg, color: t.text },
  pwdWrap: { flexDirection: "row", alignItems: "center", borderWidth: 1, borderColor: t.border, borderRadius: 10, backgroundColor: t.inputBg },
  pwdInput: { flex: 1, padding: 13, fontSize: 16, color: t.text },
  themeIcon: { fontSize: 22 },
  eyeBtn: { paddingHorizontal: 14 },
  eye: { fontSize: 20 },
  regionRow: { flexDirection: "row", flexWrap: "wrap", gap: 8, marginTop: 2 },
  regionChip: { borderWidth: 1, borderColor: t.border, backgroundColor: t.inputBg, borderRadius: 18, paddingHorizontal: 14, paddingVertical: 8 },
  regionChipOn: { backgroundColor: t.accent, borderColor: t.accent },
  regionChipT: { fontSize: 13, fontWeight: "600", color: t.sub },
  regionChipTOn: { color: "#fff" },
  regionHint: { color: t.faint, fontSize: 11.5, marginTop: 8, lineHeight: 16 },
  btn: { backgroundColor: t.accent, borderRadius: 12, padding: 16, alignItems: "center", marginTop: 26 },
  btnT: { color: "#fff", fontSize: 16, fontWeight: "800" },
  note: { color: t.faint, fontSize: 12, marginTop: 24, lineHeight: 18 },
});
